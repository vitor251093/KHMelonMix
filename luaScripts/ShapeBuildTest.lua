
dofile "MelonMixShapes.lua"
local hudScale = 5
local SCREEN_SCALE = 6.0
local scale = 192.0/(192 - 31 + 48);
local aspectRatio = 1.1
print("aspectRatio"..aspectRatio)
print("aspectRatio*aspectRatio"..(aspectRatio*aspectRatio))
Shapes2D = {}
i=0



--Test each method in parralel
Push2DShapeData(ShapeBuilder2D:square()
        :fromBottomScreen()
        :build(aspectRatio)
)

--1
Push2DShapeData(ShapeBuilder2D:square()
        :placeAtCorner(_corner.TopLeft)
        :build(aspectRatio)
)
--2   
Push2DShapeData(ShapeBuilder2D:square()
        :sourceScale(scale)
        :build(aspectRatio)
)
--3
Push2DShapeData(ShapeBuilder2D:square()
        :sourceScale(1000.0,scale)
        :build(aspectRatio)
)
--4
Push2DShapeData(ShapeBuilder2D:square()
        :hudScale(hudScale)
        :build(aspectRatio)
)
--5
Push2DShapeData(ShapeBuilder2D:square()
        :preserveDsScale()
        :build(aspectRatio)
)
--6
Push2DShapeData(ShapeBuilder2D:square()
        :fromPosition(251, 144)
        :build(aspectRatio)
)
--7
Push2DShapeData(ShapeBuilder2D:square()
        :withSize(256, 48)
        :build(aspectRatio)
)
--8
Push2DShapeData(ShapeBuilder2D:square()
        :withMargin(3.0, 6.0, 0.0, 0.0)
        :build(aspectRatio)
)
--9
Push2DShapeData(ShapeBuilder2D:square()
        :withMargin(3.0, 6.0, 0.0, 0.0)
        :build(aspectRatio)
)
--10
Push2DShapeData(ShapeBuilder2D:square()
        :withMargin(0.0, 0.0, 8.0, 4.0)
        :build(aspectRatio)
)
--11
Push2DShapeData(ShapeBuilder2D:square()
        :fadeBorderSize(5.0, 5.0, 5.0, 5.0)
        :build(aspectRatio)
)
--12
Push2DShapeData(ShapeBuilder2D:square()
        :invertGrayScaleColors()
        :build(aspectRatio)
)
--13
Push2DShapeData(ShapeBuilder2D:square()
        :repeatAsBackground()
        :build(aspectRatio)
)
--14
Push2DShapeData(ShapeBuilder2D:square()
        :repeatAsBackgroundVertically()
        :build(aspectRatio)
)
--15
Push2DShapeData(ShapeBuilder2D:square()
        :force()
        :build(aspectRatio)
)
--16
Push2DShapeData(ShapeBuilder2D:square()
        :mirror(_mirror.X)
        :build(aspectRatio)
)
--17
Push2DShapeData(ShapeBuilder2D:square()
        :cropSquareCorners(0.0, 4.0, 0.0, 0.0)
        :build(aspectRatio)
)
--18
Push2DShapeData(ShapeBuilder2D:square()
        :squareBorderRadius(10.0, 10.0, 5.0, 5.0)
        :build(aspectRatio)
)
--19
Push2DShapeData(ShapeBuilder2D:square()
        :squareBorderRadius(7.0)
        :build(aspectRatio)
)
--20
Push2DShapeData(ShapeBuilder2D:square()
        :opacity(0.90)
        :build(aspectRatio)
)
--21
Push2DShapeData(ShapeBuilder2D:square()
        :colorToAlpha(0x8, 0x30, 0xaa)
        :build(aspectRatio)
)
--22
Push2DShapeData(ShapeBuilder2D:square()
        :singleColorToAlpha(0x8, 0x30, 0xaa)
        :build(aspectRatio)
)
--23
Push2DShapeData(ShapeBuilder2D:square()
        :singleColorToAlpha(0x8, 0x30, 0xaa, 0.9)
        :build(aspectRatio)
)
--24 (Example Test shapes)
Push2DShapeData(ShapeBuilder2D:square()
        :fromPosition(100, 0)
        :withSize(20, 16)
        :placeAtCorner(_corner.TopRight)
        :sourceScale(1000.0, scale)
        :hudScale(hudScale)
        :preserveDsScale()
        :build(aspectRatio)
)
--25
Push2DShapeData(ShapeBuilder2D:square()
        :fromBottomScreen()
        :fromPosition(8, 32)
        :withSize(240, 104)
        :placeAtCorner(_corner.Center)
        :sourceScale(1.7)
        :fadeBorderSize(5.0, 5.0, 5.0, 5.0)
        :opacity(0.90)
        :hudScale(hudScale)
        :build(aspectRatio)
)
--26
Push2DShapeData(ShapeBuilder2D:square()
        :fromBottomScreen()
        :fromPosition(5, 166)
        :withSize(119, 3)
        :placeAtCorner(_corner.TopLeft)
        :withMargin(131.0, 6.0, 0.0, 0.0)
        :cropSquareCorners(0.0, 4.0, 0.0, 0.0)
        :mirror(_mirror.X)
        :hudScale(hudScale)
        :build(aspectRatio)
)

--3D Shape Test

--0
Push3DShapeData(ShapeBuilder3D:square()
        :polygonAttributes(1058996416)
        :build(aspectRatio)
)
--1
Push3DShapeData(ShapeBuilder3D:square()
        :negatePolygonAttributes(2031808)
        :build(aspectRatio)
)
--2
Push3DShapeData(ShapeBuilder3D:square()
        :color(0x0830AA)
        :build(aspectRatio)
)
--3
Push3DShapeData(ShapeBuilder3D:square()
        :negateColor(0xFFFFFF)
        :build(aspectRatio)
)
--4
Push3DShapeData(ShapeBuilder3D:square()
        :textureParam(777777)
        :build(aspectRatio)
)
--5
Push3DShapeData(ShapeBuilder3D:square()
        :negatedTextureParam(942331720)
        :build(aspectRatio)
)
--6
Push3DShapeData(ShapeBuilder3D:square()
        :polygonMode()
        :build(aspectRatio)
)
--7
Push3DShapeData(ShapeBuilder3D:square()
        :adjustAspectRatioOnly()
        :build(aspectRatio)
)
--8
Push3DShapeData(ShapeBuilder3D:square()
        :polygonVertexesCount(4)
        :build(aspectRatio)
)
--9
Push3DShapeData(ShapeBuilder3D:square()
        :placeAtCorner(_corner.BottomLeft)
        :build(aspectRatio)
)
--10
Push3DShapeData(ShapeBuilder3D:square()
        :zRange(-1.0, -0.000001)
        :build(aspectRatio)
)
--11
Push3DShapeData(ShapeBuilder3D:square()
        :zValue(0.5)
        :build(aspectRatio)
)
--12
Push3DShapeData(ShapeBuilder3D:square()
        :sourceScale(1.5)
        :build(aspectRatio)
)
--13
Push3DShapeData(ShapeBuilder3D:square()
        :sourceScale(aspectRatio*aspectRatio, 1.0)
        :build(aspectRatio)
)
--14
Push3DShapeData(ShapeBuilder3D:square()
        :hudScale(SCREEN_SCALE)
        :build(aspectRatio)
)
--15
Push3DShapeData(ShapeBuilder3D:square()
        :preserveDsScale()
        :build(aspectRatio)
)
--16
Push3DShapeData(ShapeBuilder3D:square()
        :fromPosition(128, 140)
        :build(aspectRatio)
)
--17
Push3DShapeData(ShapeBuilder3D:square()
        :withSize(100, 16)
        :build(aspectRatio)
)
--18
Push3DShapeData(ShapeBuilder3D:square()
        :withMargin(10.0, 0.0, 0.0, 0.5)
        :build(aspectRatio)
)
--19
Push3DShapeData(ShapeBuilder3D:square()
        :hide()
        :build(aspectRatio)
)
--20
Push3DShapeData(ShapeBuilder3D:square()
        :logger()
        :build(aspectRatio)
)
--21 (Example Test shapes)    
Push3DShapeData(ShapeBuilder3D:square()
        :withSize(256, 192)
        :placeAtCorner(_corner.Center)
        :sourceScale(aspectRatio*aspectRatio, 1.0)
        :hudScale(SCREEN_SCALE)
        :build(aspectRatio)
)
--22
Push3DShapeData(ShapeBuilder3D:square()
        :fromPosition(128, 140)
        :withSize(128, 52)
        :placeAtCorner(_corner.BottomRight)
        :zRange(-1.0, 0.0)
        :hudScale(hudScale)
        :build(aspectRatio)
)
--23
Push3DShapeData(ShapeBuilder3D:square()
        :polygonMode()
        :polygonAttributes(2031808)
        :fromPosition(0, 69)
        :withSize(80, 124)
        :placeAtCorner(_corner.BottomLeft)
        :withMargin(10.0, 0.0, 0.0, 0.5)
        :zRange(-1.0, -1.0)
        :negateColor(0xFFFFFF)
        :hudScale(hudScale)
        :build(aspectRatio)
)

res = Test_run_ShapeBuilderTests()
if res == 1 then 
    print("All 2D/3D ShapeBuilder Test passed!")
else
    print("At least one of the test faild... check console for more info...")
end
