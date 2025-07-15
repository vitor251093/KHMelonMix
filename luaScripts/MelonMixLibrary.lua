Mix = {}

ClearGameCodes()

dofile "MelonMixLibrary/MelonMixShapes.lua"
dofile "MelonMixLibrary/MelonMixInput.lua"

function _Update()
    Mix.CurFrameFlags = Mix.initFlags()
    Update()

end 
