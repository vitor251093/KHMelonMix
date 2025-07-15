
function Mix.initFlags() 
    return {
        HeldMask = nil,
        PressedMask = {}
    } 
end
--check if key is currently held on this frame
function KeyHeld(keyStr)
    if Mix.CurFrameFlags.HeldMask == nil then Mix.CurFrameFlags.HeldMask = HeldKeys() end
    return Mix.CurFrameFlags.HeldMask[string.byte(keyStr:sub(1,1))]
end

local PressedKeys = {}
--check if key is held on this frame but was not last frame
function KeyPress(keyStr)
    if Mix.CurFrameFlags.PressedMask[keyStr] == nil then 
        if KeyHeld(keyStr) then 
            local allreadyPressed = PressedKeys[keyStr] -- false/nil, or true
            PressedKeys[keyStr] = true
            Mix.CurFrameFlags.PressedMask[keyStr] = not allreadyPressed 
        else
            PressedKeys[keyStr] = false
            Mix.CurFrameFlags.PressedMask[keyStr] = false
        end
    end

    return Mix.CurFrameFlags.PressedMask[keyStr]
end