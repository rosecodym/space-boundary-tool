open System.IO
open System.Text.RegularExpressions

let major = 1
let minor = 4
let revision = 4

let shortString = sprintf "%i.%i.%i" major minor revision
let fullString = sprintf "%i.%i.%i.0" major minor revision

let solutionDir = @"C:/Users/Cody/Documents/Visual Studio 2010/Projects/space-boundary-tool"

let buildNeedle prefix suffix =
    sprintf @"(?<prefix>%s)\S+(?<suffix>%s)" (Regex.Escape(prefix)) (Regex.Escape(suffix))

let operate (localDir, filename, prefix, suffix, useFull) =
    let path = Path.Combine(solutionDir, localDir, filename)
    let lines = File.ReadAllLines(path)
    let needle = buildNeedle prefix suffix
    let newVersion = if useFull then fullString else shortString
    let replacement = sprintf @"${prefix}%s${suffix}" newVersion
    use writer = new StreamWriter(path)
    lines
        |> Seq.map (fun line -> Regex.Replace(line, needle, replacement))
        |> Seq.iter (fun line -> writer.WriteLine(line))

let cases = [
    ("Ifc Adapter/src", "add_to_model.cpp", "app.put(\"Version\", \"", "\");", false)
    ]

cases |> List.iter operate