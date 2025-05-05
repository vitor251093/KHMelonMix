local enum = function(keys)
    local Enum = {}
    for index, value in ipairs(keys) do
        Enum[value] = index-1
    end
    return Enum
end

local ShapeCall = enum {
    "fromPosition",
    "withSize",
    "placeAtCorner",
    "withMargin",
    "sourceScale",
    "fadeBorderSize",
    "opacity",
    "invertGrayScaleColors",
    "hudScale",
    "build", --//not implemented
    "fromBottomScreen"
}

local Corner = enum {
    "PreservePosition",
    "Center",
    "TopLeft",
    "Top",
    "TopRight",
    "Right",
    "BottomRight",
    "Bottom",
    "BottomLeft",
    "Left"
};

print("Building Shapes\n")

hudScale = 5

-- Create a shape object and store a refrence to it
Shape1 = MakeShape {
    {ShapeCall.fromBottomScreen},
    {ShapeCall.fromPosition,0, 0},
    {ShapeCall.withSize,256, 192},
    {ShapeCall.placeAtCorner,Corner.TopLeft},
    --{ShapeCall.withMargin,0.0, 30.0, 9.0, 0.0},
    {ShapeCall.sourceScale,1.5},
    --{ShapeCall.fadeBorderSize,5.0, 5.0, 5.0, 5.0},
    --{ShapeCall.opacity,0.85},
    --{ShapeCall.invertGrayScaleColors},
    {ShapeCall.hudScale,hudScale}
}

Shape2 = MakeShape {
    --{ShapeCall.fromBottomScreen},
    {ShapeCall.fromPosition,0,0},
    {ShapeCall.withSize,256,192},
    {ShapeCall.placeAtCorner,Corner.TopLeft},
    {ShapeCall.sourceScale,0.50},
    {ShapeCall.fadeBorderSize,0,0,2.5,2.5},
    {ShapeCall.opacity,0.66},
    {ShapeCall.hudScale,hudScale}
}

-- Set current shapes that should be loaded
SetShapes{Shape2,Shape1}

--Update Game scene to signal the shader to update
SetGameScene(3) 
--Would it be simpler to implement an "UpdateShader" function instead and just keep track of the game sceen in lua?

toggle = true
scene = 3
function _Update()
    updateFlags = {}
    local x = GetJoyStick(0)
    local y = GetJoyStick(1)
    joyStickInput(x,y)
    if KeyPress("K") then
        toggle = not(toggle) 
        scene = 3
        if toggle then 
            SetShapes{Shape2,Shape1}
        else
            scene = 2
            SetShapes{Shape1}
        end
        SetGameScene(scene)
    end
end

-- Example Script for a touch screen based joystick input, works with Dragon Quest IX 

DEADZONE = 500
CENTERX = 128
CENTERY = 110

local state={}
state.touching = 0
state.StylusX = 0
state.StylusY = 0

function joyStickInput(x,y)
    state.StylusX = math.floor(x/1500)+CENTERX
    state.StylusY = math.floor(y/1500)+CENTERY
    if math.abs(x) <=DEADZONE and math.abs(y)<=DEADZONE then 
        state.touching = 0
        NDSTapUp()
        return 
    end
    if state.touching <= 1 then 
        NDSTapDown(CENTERX,CENTERY)
        state.touching = state.touching+1
        return 
    end
    NDSTapDown(state.StylusX,state.StylusY)
end

function KeyHeld(keyStr)
    -- Only need to update mask once per frame
    if updateFlags.KeyboardCheck == nil then
        mask = HeldKeys()
        updateFlags.KeyboardCheck = true 
    end
    return mask[string.byte(keyStr:sub(1,1))]
end

Btns = {}
function KeyPress(keyStr)
    if KeyHeld(keyStr) and Btns[keyStr]  then
        Btns[keyStr] = false
        return true
    end
    if KeyHeld(keyStr) then 
        return false
    end
    Btns[keyStr] = true
    return false
end