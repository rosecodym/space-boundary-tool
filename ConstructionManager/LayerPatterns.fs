module internal ConstructionManager.LayerPatterns

open MaterialLibrary

let (|OpaqueOnly|_|) layers =
    let asOpaque = Array.map (fun layer ->
        match layer with 
        | noMapping when noMapping = Unchecked.defaultof<LibraryEntry> -> None
        | Opaque(opaque) -> Some(opaque)) layers
    if Array.exists (fun (maybe:LibraryEntryOpaque option) -> maybe.IsNone) asOpaque
    then None
    else Some(Array.choose id asOpaque)