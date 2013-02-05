// This script updates all version strings in the solution to match the
// specified one. It operates by searching for known locations of version 
// strings, so there's minimal risk of an improper change. However, this means
// that each location must be manually specified (in the 'cases' variable).

open System.IO
open System.Text.RegularExpressions

let major = 1
let minor = 5
let revision = 2

let shortString = sprintf "%i.%i.%i" major minor revision
let fullString = sprintf "%i.%i.%i.0" major minor revision

let solutionDir = 
    @"C:/Users/Cody/Documents/Visual Studio 2010/Projects/space-boundary-tool"

let buildNeedle prefix suffix =
    sprintf @"(?<prefix>%s)\S+(?<suffix>%s)" prefix suffix

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
    ("ConstructionManagement", "AssemblyInfo.fs", 
     "\[<assembly: Assembly(File)?Version\(\"", "\"\)>\]", true)
    ("GUI", "MainWindow.xaml", "<Window\.Title>Space Boundary Tool ", 
     "</Window\.Title>", false)
    ("GUI/Properties", "AssemblyInfo.cs", 
     "\[assembly: Assembly(File)?Version\(\"", "\"\)\]", true)
    ("Ifc Adapter/src", "add_to_model.cpp", 
     "app\.put\(\"Version\", \"", "\"\);", false)
    ("IfcInformationExtractor", "AssemblyInfo.cpp", 
     "\[assembly:AssemblyVersionAttribute\(\"", "\"\)\];", true)
    ("CSharpCoreWrapper/Properties", "AssemblyInfo.cs", 
     "\[assembly: Assembly(File)?Version\(\"", "\"\)\]", true)
    ]

cases |> List.iter operate