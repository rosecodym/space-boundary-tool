module MaterialLibrary.Loader

open System
open System.Collections.Generic

open LibIdf.Idd
open LibIdf.Idf

let public Load(idf:Idf, notify:Action<string>) : ICollection<LibraryEntry> =
    List<LibraryEntry>(idf.GetObjectsByType("Material", false)
        |> Seq.choose (fun obj ->
            try Some(LibraryEntry.Construct(obj))
            with | _ ->
                let objName = if String.IsNullOrWhiteSpace(obj.Name) then "<unnamed-object>" else obj.Name
                notify.Invoke(sprintf "Warning: Failed to load material library object '%s'. Check the definition in the IDF.\n" objName)
                None)) :> ICollection<LibraryEntry>