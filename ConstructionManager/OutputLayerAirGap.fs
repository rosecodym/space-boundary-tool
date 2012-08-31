namespace ConstructionManager

open LibIdf.Base
open MaterialLibrary

type internal OutputLayerAirGap (resistance:float) =
    inherit OutputLayer()
    override this.Name = sprintf "Air gap with resistance %f" resistance

    override this.AddToIdfV710(idf) =
        let obj = idf.CreateObject("Material:AirGap")
        obj.Fields.["Name"].Value <- Alpha(this.Name)
        obj.Fields.["Thermal Resistance"].Value <- Real(resistance)
