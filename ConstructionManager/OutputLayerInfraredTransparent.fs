namespace ConstructionManager

open LibIdf.Base
open MaterialLibrary

type internal OutputLayerInfraredTransparent () =
    inherit OutputLayer()
    override this.Name = "Infrared transparent material"

    override this.AddToIdfV710(idf) =
        let obj = idf.CreateObject("Material:InfraredTransparent")
        obj.Fields.["Name"].Value <- Alpha(this.Name)