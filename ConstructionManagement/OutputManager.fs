namespace ConstructionManagement

open System
open System.Collections.Generic

open MaterialLibrary
open ConstructionManagement.ModelConstructions
open OutputPatterns

type OutputManager (warnDelegate : Action<string>) =
    let mutable layers = Set.empty
    let mutable constructions = Set.empty
    let mutable variantTable: Map<string, int> = Map.empty

    let warn =
        let warnings = ref Set.empty
        fun w ->
            if not (Set.contains w !warnings) then 
                warnDelegate.Invoke(w)
                warnings := Set.add w !warnings

    let retrieveConstruction materials humanReadableName =
        let getVariant = fun ident -> Map.tryFind ident variantTable
        let newC = OutputConstruction(materials, humanReadableName, getVariant)
        Seq.iter warn newC.Warnings
        constructions <- Set.add newC constructions
        newC

    let retrieveLayer (layer:OutputLayer) =
        layers <- layers.Add(layer)
        layer

    let retrieveOpaqueLayer (libraryEntry:LibraryEntryOpaque) thickness =
        let n = sprintf "%s (%.3f)" (libraryEntry.Name.ToString()) thickness
        let name =
            if n.Length <= 100 
            then n 
            else sprintf "Material id %i" (n.GetHashCode())
        retrieveLayer (OutputLayerOpaque(name, libraryEntry, thickness))

    let retrieveOpaqueSurface (libraryEntry:LibraryEntryOpaque) =
        let n = sprintf "%s (surface only)" (libraryEntry.Name.ToString())
        let name =
            if n.Length <= 100
            then n
            else sprintf "Material id %i" (n.GetHashCode())
        retrieveLayer (OutputLayerOpaque(name, libraryEntry, 0.001))

    let retrieveCopy (newThickness: double option) (libraryEntry:LibraryEntry) =
        match libraryEntry, newThickness with
        | LibraryEntry.AirGap(props), _ -> retrieveLayer (OutputLayerAirGap(float props.ThermalResistance))
        | LibraryEntry.Composite(_), _ -> raise (ArgumentException())
        | LibraryEntry.Gas(props), _ -> retrieveLayer (OutputLayerGas(props.Name.ToString(), props, float props.Thickness))
        | LibraryEntry.Glazing(props), _ -> retrieveLayer (OutputLayerGlazing(props.Name.ToString(), props, float props.Thickness))
        | LibraryEntry.NoMass(props), _ -> retrieveLayer (OutputLayerNoMass(props.Name.ToString(), props))
        | LibraryEntry.Opaque(props), Some(t) -> retrieveOpaqueLayer props t
        | LibraryEntry.Opaque(props), None -> retrieveOpaqueLayer props (float props.Thickness)

    let cloneLibraryComposite withName layers =
        let layerCopies = layers |> List.map (retrieveCopy None)
        retrieveConstruction layerCopies withName

    let rec partial layers depth soFar =
        match layers, depth with
        | (nextE, nextT) :: rest, d when d >= nextT ->
            partial rest (d - nextT) (soFar @ [nextE, nextT])
        | (nextE, nextT) :: rest, d when d < nextT ->
            soFar @ [nextE, d]
        | _ -> soFar
        // The last case can be reached either in the case that the program is
        // in a state that it shouldn't be in or the case that fp imprecision
        // has mucked something up. I don't think it's worth trying to
        // eliminate the fp issue.

    let retrieveModifiedComposite origName layers depth reversed =
        let fromReversed = if reversed then List.rev layers else layers
        let fromDepth =
            match depth with
            | Some(d) -> partial fromReversed d []
            | None -> fromReversed
        let outputLayers =
            fromDepth |> List.map (fun (e, t) -> retrieveCopy (Some(t)) e)
        retrieveConstruction outputLayers origName

    // For some reason, rewriting this point-free borks the autogeneralization.
    let getThickestLayer layers = Seq.maxBy snd layers |> fst

    let areParallel = ModelConstruction.normalsParallel
    let areAntiparallel = ModelConstruction.normalsAntiparallel

    member this.AllOutputLayers = layers :> IEnumerable<OutputLayer>
    member this.AllOutputConstructions =
        constructions :> IEnumerable<OutputConstruction>

    member this.ConstructionForLayers(surfaceNormal,
                                      constructions, 
                                      normals, 
                                      thicknesses) : OutputConstructionBase =
        let emitProblemConstruction (c: ProblemConstruction) =
            warn c.Message
            c :> OutputConstructionBase
        let cnorms = 
            normals 
            |> Seq.map (function
                | 0.0, 0.0, 0.0 -> None
                | n -> Some(n))
            |> List.ofSeq
        match List.ofSeq constructions with
        | Empty | SingleAirSpace -> 
            let layer = retrieveLayer (OutputLayerInfraredTransparent())
            upcast retrieveConstruction ([layer]) None
        | MappedWindow(name, libraryLayers) -> 
            upcast cloneLibraryComposite (Some(name)) libraryLayers 
        | SimpleOnly(infos) -> 
            let ts = thicknesses |> Seq.map (fun t -> Some(t)) |> List.ofSeq
            let outputLayers = List.map2 retrieveCopy ts infos
            upcast retrieveConstruction outputLayers None
        | SingleComposite(name, layers) ->
            let reqThickness = Seq.head thicknesses
            let compositeNorm = cnorms.[0]
            if compositeNorm.IsNone then
                emitProblemConstruction (UnorientedComposite()) else
            if not (areParallel surfaceNormal compositeNorm.Value) then
                let thickest = getThickestLayer layers
                let layer = retrieveCopy (Some(reqThickness)) thickest
                upcast retrieveConstruction [layer] None else
            let reversed = areAntiparallel surfaceNormal compositeNorm.Value
            // Don't use the requested thickness here, in case it's not quite
            // equal to the total composite thickness. (If this happens and the
            // composite is reversed somewhere EnergyPlus will fatally
            // complain.)
            let c = retrieveModifiedComposite name layers None reversed
            upcast c
        | TwoComposites surfaceNormal cnorms (name, comp1, comp2) ->
            let t1 = Seq.nth 0 thicknesses
            let t2 = Seq.nth 1 thicknesses
            let entries = (partial comp1 t1 []) @ (partial comp2 t2 [])
            let getLayer (entry, t) = retrieveCopy (Some(t)) entry
            upcast 
                retrieveConstruction (List.map getLayer entries) (Some(name))
        | _ ->
            emitProblemConstruction (UnknownProblemComposite())

    member this.ConstructionForSurface (surfaceNormal,
                                        constructionNormal,
                                        c) : OutputConstructionBase = 
        let emitProblemConstruction (c: ProblemConstruction) =
            warn c.Message
            c :> OutputConstructionBase
        match c with
        | ModelConstruction.SingleOpaque(src) ->
            match src.MappingTarget with
            | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> 
                emitProblemConstruction (BadMappingConstruction())
            | LibraryEntry.AirGap(_) ->
                let layer = retrieveLayer (OutputLayerInfraredTransparent())
                upcast retrieveConstruction ([layer]) None
            | LibraryEntry.Opaque(entry) -> 
                let surfaceLayer = retrieveOpaqueSurface entry
                upcast retrieveConstruction [surfaceLayer] None
            | LibraryEntry.Composite(_) -> 
                emitProblemConstruction (LibraryComposite())
            | _ -> emitProblemConstruction (BadMappingConstruction())
        | ModelConstruction.Window(_) -> 
            emitProblemConstruction (AdiabaticWindowConstruction())
        | ModelConstruction.LayerSet(_, layers) ->
            let relevantLayer =
                if not (areParallel surfaceNormal constructionNormal) then
                    layers
                    |> Seq.filter (fun (src, _) ->
                        match src.MappingTarget with
                        | LibraryEntry.AirGap(_) -> false
                        | _ -> true)
                    |> getThickestLayer
                else if areAntiparallel surfaceNormal constructionNormal then
                    layers |> List.rev |> Seq.head |> fst
                else
                    layers.Head |> fst
            match relevantLayer.MappingTarget with
            | noMap when noMap = Unchecked.defaultof<LibraryEntry> ->
                emitProblemConstruction (BadMappingConstruction())
            | LibraryEntry.Opaque(entry) ->
                let surfaceLayer = retrieveOpaqueSurface entry
                upcast retrieveConstruction [surfaceLayer] None
            | e ->
                // I don't know what this case is for.
                let res = retrieveConstruction [retrieveCopy None e] None
                upcast res
            | _ -> emitProblemConstruction (BadMappingConstruction())

    member this.PruneOutput (against: seq<OutputConstructionBase>) =
        let newConstructions =
            against |>
            Seq.choose (function
                        | :? OutputConstruction as c -> Some(c)
                        | _ -> None) |>
            Set.ofSeq
        if not (Set.isSuperset constructions newConstructions) then
            raise (ArgumentException("A construction OutputManager should \
                                      only be pruned against constructions \
                                      from that manager."))
        constructions <- newConstructions
        let newLayerNames =
            newConstructions |>
            Seq.collect (fun c -> c.LayerNames) |>
            Set.ofSeq
        layers <-
            layers |>
            Set.filter (fun layer -> newLayerNames.Contains(layer.Name))

    member this.IdentifyConstructionVariants () =
        variantTable <-
            constructions |>
            Seq.groupBy (fun c -> c.InvariantName) |>
            Seq.map snd |>
            Seq.filter (Seq.length >> (<>) 1) |>
            Seq.collect (Seq.mapi (fun v c -> c.Identifier, v)) |>
            Map.ofSeq