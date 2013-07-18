#nowarn "25"

open System.IO

if fsi.CommandLineArgs.Length < 4 then () else
let [|command; keyFilePath; targetPath|] = fsi.CommandLineArgs.[1..3]
if command <> "--inject" && command <> "--remove" then () else
let licenseKey = File.ReadAllLines(keyFilePath).[0]
let targetFileLines = File.ReadAllLines(targetPath)
use writer = new StreamWriter(targetPath)
targetFileLines
|> Seq.map (fun line -> 
    if command = "--inject" then line.Replace("EDM LICENSE KEY", licenseKey)
    else line.Replace(licenseKey, "EDM LICENSE KEY"))
|> Seq.iter writer.WriteLine