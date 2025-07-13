
dofile "MelonMixLibrary.lua"

--AddGameCode appends the given u32 gamecode to the list of roms that the plugin manager will load the luaPlugins shader for.
AddGameCode(0xFFFFFFFF) --gamecode 0xFFFFFFFF means *any* game

local hudScale = 5
local aspectRatio = 1.0
local HUDShape = Push2DShapeData(ShapeBuilder2D:square()
    :fromBottomScreen()
    :fromPosition(0,0)
    :withSize(256,192)
    :placeAtCorner(_corner.TopLeft)
    :sourceScale(1.5)
    :hudScale(hudScale)
    :build(aspectRatio)
)
local MiniMapShape = Push2DShapeData(ShapeBuilder2D:square()
    :fromPosition(0,0)
    :withSize(256,192)
    :placeAtCorner(_corner.TopLeft)
    :sourceScale(0.75)
    :fadeBorderSize(0,0,2.5,2.5)
    :opacity(0.66)
    :hudScale(hudScale)
    :build(aspectRatio)
)
local Screen3D  = Push3DShapeData(ShapeBuilder3D:square()
    :fromPosition(0,0)
    :withSize(256,192)
    :placeAtCorner(_corner.Center)
    :sourceScale(0.75)
    :hudScale(hudScale)
    :build(aspectRatio)
)

-- Set current shapes that should be loaded
Set2DShapes{MiniMapShape,HUDShape}
Set3DShapes{Screen3D}

--Update Game scene to signal the shader to update
SetGameScene(3)
local miniMapIsup = true

function toggleMiniMap() --Function to toggle showing the bottom Screen as a miniMap
    miniMapIsup = not miniMapIsup 
    if miniMapIsup then
        Set2DShapes{MiniMapShape,HUDShape}
        SetGameScene(3)
    else
        Set2DShapes{HUDShape}
        SetGameScene(2)
    end
end

--_Update is called once every frame by MelonDS
function Update()
    if KeyPress("K") then 
        toggleMiniMap() 
    end
    if KeyPress("Q") then 
        print(GetGameCode() or "none") --Prints the currently loaded rom's gamecode 
    end
end



