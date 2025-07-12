
local SCREEN_SCALE = 6.0

--utility function to implement enumeration
local enum = function(keys)
    local Enum = {}
    for index, value in ipairs(keys) do
        Enum[value] = index-1
    end
    return Enum
end

_shape = enum{
    "Square"
}

_corner = enum{
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
}

_mirror = {
    None = 0x00,
    X = 0x80,
    Y = 0x10,
    XY = 0x18
}

--utility function to help implement c-like structs / classes
local function newStruct(self,init)
    local newStruct = init or {}
    setmetatable(newStruct,self)
    self.__index = self
    return newStruct
end

--generic vector object, with "bytes" method to convert to packed data string 
local vector = {}
function vector:new(init)
    return newStruct(self,init)
end
function vector:bytes()
    error("Bytes function not found for this vector...")
end

local vec2 = vector:new({x=0.0,y=0.0})
function vec2:bytes()
    return string.pack("ff",self.x,self.y)
end
local ivec4 = vector:new({x=0,y=0,z=0,w=0})
function ivec4:bytes()
    return string.pack("iiii",self.x,self.y,self.z,self.w)
end
local vec4 = vector:new({x=0.0,y=0.0,z=0.0,w=0.0})
function vec4:bytes()
    return string.pack("ffff",self.x,self.y,self.z,self.w)
end

local iarr4 = vector:new({[0]=0,0,0,0}) --NOTE: 0 indexed
function iarr4:bytes()
    return string.pack("iiii",self[0],self[1],self[2],self[3])
end

local uiarr4 = vector:new({[0]=0,0,0,0}) --NOTE: 0 indexed
function uiarr4:bytes()
    return string.pack("IIII",self[0],self[1],self[2],self[3])
end

local ShapeData2D = {}
function ShapeData2D:new()
    local init = {
        sourceScale = vec2:new({x=1.0,y=1.0}),
        
        effects = 0,
        -- 0x01 => invertGrayScaleColors
        -- 0x02 => crop corner as triangle
        -- 0x04 => rounded corners
        -- 0x08 => mirror X
        -- 0x10 => mirror Y
        -- 0x20 => manipulate transparency
        
        opacity = 1.0,
        
        squareInitialCoords = ivec4:new({x=0,y=0,z=256,w=192}),
        squareFinalCoords = vec4:new(),
        
        fadeBorderSize = vec4:new(),
        
        squareCornersModifier = vec4:new(),
        
        colorToAlpha = ivec4:new(),
        singleColorToAlpha = ivec4:new()
    }
    return newStruct(self,init)
end

--UBO-compatible format
function ShapeData2D:bytes() 
    return table.concat({
        self.sourceScale:bytes(),-- 8 bytes (X factor, Y factor)
        string.pack("i",self.effects),
        string.pack("f",self.opacity),
        self.squareInitialCoords:bytes(),-- 16 bytes (X, Y, Width, Height)
        self.squareFinalCoords:bytes(),-- 16 bytes (X, Y, Width, Height)
        self.fadeBorderSize:bytes(),-- 16 bytes (left fade, top fade, right fade, down fade)
        self.squareCornersModifier:bytes(),-- 16 bytes (top left, top right, bottom left, bottom right)
        self.colorToAlpha:bytes(),-- 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
        self.singleColorToAlpha:bytes() -- 16 bytes (RGBA, and the A acts as an enabled/disabled toggle)
    })
end


local ShapeData3D = {}
function ShapeData3D:new()
    local init = {
        sourceScale = vec2:new({x=1.0,y=1.0}),
        
        corner = _corner.PreservePosition,
        hudScale = SCREEN_SCALE,
        
        squareInitialCoords = ivec4:new({x=0,y=0,z=256,w=192}),
        
        margin = vec4:new(),
        
        zRange = vec2:new({x=-1.0,y=1.0}),
        polygonVertexesCount = 0,
        
        effects = 0,
        -- 0x01 => polygon mode
        -- 0x02 => hide

        polygonAttributes = uiarr4:new(),
        negatedPolygonAttributes = uiarr4:new(),

        color = iarr4:new(),
        negatedColor = iarr4:new(),

        textureParams = uiarr4:new(),
        negatedTextureParams = uiarr4:new(),

        colorCount = 0,
        negatedColorCount = 0,
        textureParamCount = 0,
        negatedTextureParamCount =0
    } 
    return newStruct(self,init)
end

function ShapeData3D:bytes() 
    return table.concat({
        self.sourceScale:bytes(), -- 8 bytes (X factor, Y factor)
        string.pack("i",self.corner), -- 4 bytes
        string.pack("f",self.hudScale), -- 4 bytes
        self.squareInitialCoords:bytes(), -- 16 bytes (X, Y, Width, Height)
        self.margin:bytes(), -- 16 bytes (left, top, right, bottom)
        self.zRange:bytes(), -- 8 bytes
        string.pack("i",self.polygonVertexesCount), -- 4 bytes
        string.pack("i",self.effects), -- 4 bytes
        self.polygonAttributes:bytes(), -- 16 bytes
        self.negatedPolygonAttributes:bytes(), -- 16 bytes
        self.color:bytes(), -- 16 bytes
        self.negatedColor:bytes(), -- 16 bytes
        self.textureParams:bytes(), --16 bytes
        self.negatedTextureParams:bytes(), -- 16 bytes
        string.pack("i",self.colorCount), -- 4 bytes
        string.pack("i",self.negatedColorCount), -- 4 bytes
        string.pack("i",self.textureParamCount), -- 4 bytes
        string.pack("i",self.negatedTextureParamCount)
    })
end

ShapeBuilder2D ={}
function ShapeBuilder2D:square()
    return newStruct(self,{
        shapeData           = ShapeData2D:new(),
        _shape              = _shape.Square,
        _fromBottomScreen   = false,
        _corner             = _corner.PreservePosition,
        _hudScale           = 1.0,
        _margin             = vec4:new()
    })
end

function ShapeBuilder2D:fromBottomScreen()
    self._fromBottomScreen = true
    return self
end

function ShapeBuilder2D:placeAtCorner(corner)
    self._corner = corner
    return self
end

function ShapeBuilder2D:sourceScale(scalex,scaley)
    if scaley == nil then scaley = scalex end
    self.shapeData.sourceScale.x = scalex
    self.shapeData.sourceScale.y = scaley
    return self
end

function ShapeBuilder2D:hudScale(hudScale)
    self._hudScale = hudScale
    return self
end

function ShapeBuilder2D:preserveDsScale()
    local mulScale = SCREEN_SCALE/self._hudScale
    self.shapeData.sourceScale.x = self.shapeData.sourceScale.x * mulScale
    self.shapeData.sourceScale.y = self.shapeData.sourceScale.y * mulScale
    return self
end 

function ShapeBuilder2D:fromPosition(x,y)
    if self._shape == _shape.Square then 
        self.shapeData.squareInitialCoords.x = x
        self.shapeData.squareInitialCoords.y = y
    end
    return self
end

function ShapeBuilder2D:withSize(width,height)
    if self._shape == _shape.Square then 
        self.shapeData.squareInitialCoords.z = width
        self.shapeData.squareInitialCoords.w = height
    end
    return self
end

function ShapeBuilder2D:withMargin(left,top,right,bottom)
    self._margin.x = left
    self._margin.y = top
    self._margin.z = right
    self._margin.w = bottom
    return self
end

function ShapeBuilder2D:fadeBorderSize(left,top,right,bottom)
    self.shapeData.effects = self.shapeData.effects | 0x20
    self.shapeData.fadeBorderSize.x = left
    self.shapeData.fadeBorderSize.y = top
    self.shapeData.fadeBorderSize.z = right
    self.shapeData.fadeBorderSize.w = bottom
    return self
end

function ShapeBuilder2D:invertGrayScaleColors()
    self.shapeData.effects = self.shapeData.effects | 0x1
    return self
end

function ShapeBuilder2D:repeatAsBackground()
    self.shapeData.effects = self.shapeData.effects | 0x40 | 0x80
    return self
end

function ShapeBuilder2D:repeatAsBackgroundHorizontally()
    self.shapeData.effects = self.shapeData.effects | 0x40
    return self
end

function ShapeBuilder2D:repeatAsBackgroundVertically()
    self.shapeData.effects = self.shapeData.effects | 0x80
    return self
end

function ShapeBuilder2D:force()
    self.shapeData.effects = self.shapeData.effects | 0x100
    return self
end

function ShapeBuilder2D:mirror(mirror)
    self.shapeData.effects = self.shapeData.effects | mirror
    return self
end

function ShapeBuilder2D:cropSquareCorners(topLeft,topRight,bottomLeft,bottomRight)
    self.shapeData.effects = self.shapeData.effects | 0x2
    self.shapeData.squareCornersModifier.x = topLeft
    self.shapeData.squareCornersModifier.y = topRight
    self.shapeData.squareCornersModifier.z = bottomLeft
    self.shapeData.squareCornersModifier.w = bottomRight
    return self
end

function ShapeBuilder2D:squareBorderRadius(topLeft,topRight,bottomLeft,bottomRight)
    --if only first argument given set all other arguments to the first one...
    if topRight == nil then topRight = topLeft end
    if bottomLeft == nil then bottomLeft = topLeft end
    if bottomRight == nil then bottomRight = topLeft end

    self.shapeData.effects = self.shapeData.effects | 0x4
    self.shapeData.squareCornersModifier.x = topLeft
    self.shapeData.squareCornersModifier.y = topRight
    self.shapeData.squareCornersModifier.z = bottomLeft
    self.shapeData.squareCornersModifier.w = bottomRight
    return self
end

function ShapeBuilder2D:opacity(opacity)
    self.shapeData.effects = self.shapeData.effects | 0x20
    self.shapeData.opacity = opacity
    return self
end

function ShapeBuilder2D:colorToAlpha(red,green,blue)
    self.shapeData.effects = self.shapeData.effects | 0x20
    self.shapeData.colorToAlpha.x = red >> 2
    self.shapeData.colorToAlpha.y = green >> 2
    self.shapeData.colorToAlpha.z = blue >> 2
    self.shapeData.colorToAlpha.w = 1
    return self
end

function ShapeBuilder2D:singleColorToAlpha(red,green,blue)
    self.shapeData.effects = self.shapeData.effects | 0x20
    self.shapeData.singleColorToAlpha.x = red >> 2
    self.shapeData.singleColorToAlpha.y = green >> 2
    self.shapeData.singleColorToAlpha.z = blue >> 2
    self.shapeData.singleColorToAlpha.w = 1
    return self
end

function ShapeBuilder2D:precompute3DCoordinatesOf2DSquareShape(aspectRatio)
    local iuTexScale = SCREEN_SCALE/self._hudScale

    local ScreenWidth = 256.0*iuTexScale
    local ScreenHeight = 192.0*iuTexScale

    local scaleX = self.shapeData.sourceScale.x
    local scaleY = self.shapeData.sourceScale.y

    local heightScale = 1.0/aspectRatio

    local squareFinalWidth = self.shapeData.squareInitialCoords.z*scaleX*heightScale
    local squareFinalHeight = self.shapeData.squareInitialCoords.w*scaleY

    local squareFinalX1 = 0.0
    local squareFinalY1 = 0.0

    local function switch(condition,cases) return cases[condition]() end

    switch(self._corner,
    {
        [_corner.PreservePosition] = function()
            squareFinalX1 = (self.shapeData.squareInitialCoords.x + self.shapeData.squareInitialCoords.z/2)*scaleX - squareFinalWidth/2
            squareFinalY1 = (self.shapeData.squareInitialCoords.y + self.shapeData.squareInitialCoords.w/2)*scaleY - squareFinalHeight/2
        end,
        [_corner.Center] = function()
            squareFinalX1 = (ScreenWidth - squareFinalWidth)/2
            squareFinalY1 = (ScreenHeight - squareFinalHeight)/2
        end,
        [_corner.TopLeft] = function()
        end,
        [_corner.Top] = function()
            squareFinalX1 = (ScreenWidth - squareFinalWidth)/2
        end,
        [_corner.TopRight] = function()
            squareFinalX1 = ScreenWidth - squareFinalWidth
        end,
        [_corner.Right] = function()
            squareFinalX1 = ScreenWidth - squareFinalWidth
            squareFinalY1 = (ScreenHeight - squareFinalHeight)/2
        end,
        [_corner.BottomRight] = function()
            squareFinalX1 = ScreenWidth - squareFinalWidth
            squareFinalY1 = ScreenHeight - squareFinalHeight
        end,
        [_corner.Bottom] = function()
            squareFinalX1 = (ScreenWidth - squareFinalWidth)/2
            squareFinalY1 = ScreenHeight - squareFinalHeight
        end,
        [_corner.BottomLeft] = function()
            squareFinalY1 = ScreenHeight - squareFinalHeight
        end,
        [_corner.Left] = function()
            squareFinalY1 = (ScreenHeight - squareFinalHeight)/2
        end,
    })

    squareFinalX1 = squareFinalX1 + (self._margin.x - self._margin.z)*heightScale
    squareFinalY1 = squareFinalY1 + (self._margin.y - self._margin.w)

    local squareFinalX2 = squareFinalX1 + squareFinalWidth
    local squareFinalY2 = squareFinalY1 + squareFinalHeight

    self.shapeData.squareFinalCoords.x = squareFinalX1
    self.shapeData.squareFinalCoords.y = squareFinalY1
    self.shapeData.squareFinalCoords.z = squareFinalX2
    self.shapeData.squareFinalCoords.w = squareFinalY2
end

function ShapeBuilder2D:build(aspectRatio)
    if self._fromBottomScreen then 
        self.shapeData.squareInitialCoords.y = self.shapeData.squareInitialCoords.y + 192
    end
    if self._shape == _shape.Square then 
        self:precompute3DCoordinatesOf2DSquareShape(aspectRatio)
    end
    return self.shapeData:bytes()
end

ShapeBuilder3D = {}
function ShapeBuilder3D:square()
    return newStruct(self,{
        shapeData = ShapeData3D:new(),
        _shape = _shape.Square,
        _aspectRatio = 0.0,
        _polygonAttributesIndex = 0,
        _negatedPolygonAttributesIndex = 0
    })
end

function ShapeBuilder3D:polygonAttributes(_polygonAttributes)
    self.shapeData.polygonAttributes[self._polygonAttributesIndex] = _polygonAttributes
    self._polygonAttributesIndex = self._polygonAttributesIndex+1
    return self
end
function ShapeBuilder3D:negatePolygonAttributes(_polygonAttributes)
    self.shapeData.negatedPolygonAttributes[self._negatedPolygonAttributesIndex] = _polygonAttributes
    self._negatedPolygonAttributesIndex = self._negatedPolygonAttributesIndex+1
    return self
end
function ShapeBuilder3D:color(_color)
    self.shapeData.color[self.shapeData.colorCount] = _color
    self.shapeData.colorCount = self.shapeData.colorCount+1
    return self
end
function ShapeBuilder3D:negateColor(_color)
    self.shapeData.negatedColor[self.shapeData.negatedColorCount] = _color
    self.shapeData.negatedColorCount = self.shapeData.negatedColorCount+1
    return self
end
function ShapeBuilder3D:textureParam(_textureParam) 
    self.shapeData.textureParams[self.shapeData.textureParamCount] = _textureParam
    self.shapeData.textureParamCount = self.shapeData.textureParamCount+1
    return self
end
function ShapeBuilder3D:negatedTextureParam(_textureParam)
    self.shapeData.negatedTextureParams[self.shapeData.negatedTextureParamCount] = _textureParam
    self.shapeData.negatedTextureParamCount = self.shapeData.negatedTextureParamCount+1
    return self
end
function ShapeBuilder3D:polygonMode()
    self.shapeData.effects = self.shapeData.effects | 0x1
    return self
end
function ShapeBuilder3D:adjustAspectRatioOnly()
    self.shapeData.effects = self.shapeData.effects | 0x8
    return self
end
function ShapeBuilder3D:polygonVertexesCount(_polygonVertexesCount)
    self.shapeData.polygonVertexesCount = _polygonVertexesCount
    return self
end
function ShapeBuilder3D:placeAtCorner(corner)
    self.shapeData.corner = corner
    return self
end
function ShapeBuilder3D:zRange(_minZ, _maxZ)
    self.shapeData.zRange.x = _minZ
    self.shapeData.zRange.y = _maxZ
    return self
end
function ShapeBuilder3D:zValue(z)
    self.shapeData.zRange.x = z
    self.shapeData.zRange.y = z
    return self
end
function ShapeBuilder3D:sourceScale(_scaleX, _scaleY)
    if _scaleY == nil then _scaleY = _scaleX end --_scaleY is optional
    self.shapeData.sourceScale.x = _scaleX
    self.shapeData.sourceScale.y = _scaleY
    return self
end
function ShapeBuilder3D:hudScale(hudScale)
    self.shapeData.hudScale = hudScale
    return self
end
function ShapeBuilder3D:preserveDsScale()
    local mulScale = SCREEN_SCALE/self.shapeData.hudScale
    self.shapeData.sourceScale.x = self.shapeData.sourceScale.x * mulScale
    self.shapeData.sourceScale.y = self.shapeData.sourceScale.y * mulScale
    return self
end
function ShapeBuilder3D:fromPosition(x, y)
    if _shape == _shape.Square then 
        self.shapeData.squareInitialCoords.x = x
        self.shapeData.squareInitialCoords.y = y
    end
    return self
end
function ShapeBuilder3D:withSize(width, height)
    if _shape == _shape.Square then 
        self.shapeData.squareInitialCoords.z = width
        self.shapeData.squareInitialCoords.w = height
    end
    return self
end
function ShapeBuilder3D:withMargin(left, top, right, bottom)
    self.shapeData.margin.x = left
    self.shapeData.margin.y = top
    self.shapeData.margin.z = right
    self.shapeData.margin.w = bottom
    return self
end
function ShapeBuilder3D:hide()
    self.shapeData.effects = self.shapeData.effects | 0x2
    return self
end
function ShapeBuilder3D:logger()
    self.shapeData.effects = self.shapeData.effects | 0x4
    return self
end

function ShapeBuilder3D:build(aspectRatio)
    self._aspectRatio = aspectRatio
    return self.shapeData:bytes()
end
