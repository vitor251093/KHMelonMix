
dofile "MelonMixShapes.lua"

local hudScale = 5
local aspectRatio = GetCurrentAspectRatio()

local shapeData_A = ShapeBuilder2D:square()
    :fromBottomScreen()
    :fromPosition(0,0)
    :withSize(256,192)
    :placeAtCorner(_corner.TopLeft)
    :sourceScale(1.5)
    :hudScale(hudScale)
    :build(1)

local shapeData_B = ShapeBuilder2D:square()
    :fromPosition(0,0)
    :withSize(256,192)
    :placeAtCorner(_corner.TopLeft)
    :sourceScale(0.75)
    :fadeBorderSize(0,0,2.5,2.5)
    :opacity(0.66)
    :hudScale(hudScale)
    :build(1)

local shapeData_C = ShapeBuilder3D:square()
    :fromPosition(0,0)
    :withSize(256,192)
    :placeAtCorner(_corner.Center)
    :sourceScale(0.75)
    :hudScale(hudScale)
    :build(1)

local HUDShape = Push2DShapeData(shapeData_A:bytes())
local MiniMapShape = Push2DShapeData(shapeData_B:bytes())
local Shape3D = Push3DShapeData(shapeData_C:bytes())
print(Shape3D)
-- Set current shapes that should be loaded
Set2DShapes{MiniMapShape,HUDShape}
print(Set3DShapes{Shape3D})

--Update Game scene to signal the shader to update
SetGameScene(3)
local miniMapIsup = true


function toggleMiniMap() --Function to toggle showing the bottom Screen as a miniMap
    miniMapIsup = not miniMapIsup 
    print("hi")
    if miniMapIsup then
        Set2DShapes{MiniMapShape,HUDShape}
        SetGameScene(3)
    else
        Set2DShapes{HUDShape}
        SetGameScene(2)
    end
end

local updateFlags = {}
--_Update is called once every frame by MelonDS    
function _Update() 
    updateFlags = {}
    if KeyPress("K") then 
        toggleMiniMap() 
    end
end


--TODO: write this logic directly into the backend so it dosen't need to be added in lua...

--true if key is currently held down
function KeyHeld(keyStr)
    if not updateFlags.HeldMask then updateFlags.HeldMask = HeldKeys() end
    return updateFlags.HeldMask[string.byte(keyStr:sub(1,1))]
end

local PressedKeys = {}
--check if key is held on this frame but was not last frame
function KeyPress(keyStr)
    if not updateFlags.PressedMask then updateFlags.PressedMask = {} end

    if updateFlags.PressedMask[keyStr] == nil then 
        if KeyHeld(keyStr) then 
            local allreadyPressed = PressedKeys[keyStr]
            PressedKeys[keyStr] = true
            updateFlags.PressedMask[keyStr] = not allreadyPressed
        else
            PressedKeys[keyStr] = false
            updateFlags.PressedMask[keyStr] = false
        end
    end

    return updateFlags.PressedMask[keyStr]
end

local PressedKeys = {}



