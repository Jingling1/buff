# buff能量机关的视觉识别
一、	灰度图的获取
先后采用YUV通道相减、HSV颜色分割，出现以下问题——能量机关周围有光晕大面积被处理为白色，能量机关颜色HSV范围跨度太大不能很好的分割。为解决以上问题，先将RGB图像转换为灰度图，再对灰度图进行幂率变换（若使用相机调节gamma值可以直接达到效果，对灰度图进行幂率变换与直接对RGB图像幂率变换相比，计算量更少），之后不再区分红蓝能量机关，能量机关获取方法见下，一般的红蓝色，或者光晕，不对识别产生影响。（其中幂率变换可使用两个API加快）

二、	能量机关扇叶的获取
一开始对整张图取反等处理后，进行漫水填充，出现了能量机关破损的情况，漫水填充失效，之后联想到连通域（扇叶虽然破损，但仍是一个连通图形），使用了连通域函数connectedComponentsWithStats，对面积进行限制，获取了能量机关扇叶区域的矩形，后来弃用此函数（太费时），直接对整张灰度图使用findContours，遍历轮廓使用boudingRect框选，通过矩形的面积进行筛选，得到能量机关扇叶矩形，也即得到了ROI区域。接下来，对ROI图像进行漫水填充，寻找轮廓，如果为full_blade（全亮扇叶）则会出现一个装甲区，两个长条状，通过面积、比例关系筛选，如果得到了一个装甲和一个及以上的长条状，则为full_blade，通过sign变量控制最后是否筛选half_blade（半亮扇叶），半亮扇叶流动区的图形会影响之后圆心的识别，如果半亮扇叶仅装甲部分被框进ROI区域则通过几何操作及图形比例得到框住装甲及流动区的矩形，通过旋转及透视变换函数将此ROI区域变为全黑；若半亮扇叶整个地被框进矩形内，则直接使用漫水填充处理后的ROI覆盖原图像相应区域（两种情况可以通过装甲区面积与ROI面积的比值确定，处理后流动区图形被消除）

三、	圆心的获取
得到各个装甲区之后通过几何操作框出包含圆心的ROI区域，通过findContours函数，加上面积比例筛选出圆心，得到圆心坐标。

四、	旋转方式的判断
在detect类的控制下，选取两帧中间间隔3帧的图像，计算角度，判断旋转方向，由于前面的圆心半径应该还比较准确，预测直接使用圆周运动，代码中将这一帧的
预测标在了下一帧的图像上，可以观察到结果。

五、	几何方面的处理
得到装甲后，由装甲出发获得圆心区域存在一个方向的问题，test1_process和test2_process为两个方向的矩形处理，整个程序中只有这两个方向。

六、	出现的问题
最开始的gamma值调节不当，造成了图形缺口，错误地将方向改为弥补图形，通过尝试发现，使用findContours的检测点结果可以比较方便地弥补图形缺口，但最终调节gamma值之后发现这一方向的努力其实没必要。这一步骤中还用到了excel编辑程序写出的txt文档，通过excel作图得到了能量机关扇叶的图形，节省了一部分的时间。

七、	接下来需要做的
识别改进，识别出需要的装甲区域之后，后续处理过程都比较方便；记录能量机关点亮顺序，按照相对位置记录；各种情况的讨论处理（二中区域的填补分了多种情况还未完成，后续再看看是否有必要分类，完善代码）；开始阅读学习两支强队的代码。
