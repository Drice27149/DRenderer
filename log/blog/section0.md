# 实时渲染器开发阶段性小结

从软渲染器到 OpenGL 再到 DX12, 目前渲染器算是有点能看的效果。

软渲染器, OpenGL 的难度曲线还算平缓, DX12可以说是摸着坑前进的。尤其是 GPU 编程不能用传统的方式调试和输出, 出现了几次一个 bug 卡住几天的情况, 靠 RenderDoc 和 PIX 一点一点磨过去了。

做的过程中也发现了自己的许多问题: 对大型项目的把控能力不足, 对并发编程没有经验等。虽然算法题做过不少了, 但大型工程能力的锻炼一直没有重视, 觉得那是 "码农" 的工作。

一个几百行的解题程序, 难度往往来自算法思路的刁钻和巧妙，一旦想通了之后实现起来会很快。

在面对一个系统级的工程时, 开发者会有较重的心智负担，因为需要在细节和全局之间不断跳跃。

### 附录0

[filament](https://github.com/google/filament)

[Real Shading in Unreal Engine 4](https://blog.selfshadow.com/publications/s2013-shading-course/#course_content)

[Image based lighting](https://drive.google.com/file/d/1N9c-zVpwEQJJ-dSeSbQXKhms-h66QJ6S/view)

[Practical Clustered Shading - Humus](http://www.humus.name/Articles/PracticalClusteredShading.pdf)

[HIGH-QUALITY TEMPORAL SUPERSAMPLING](https://de45xmedrsdbp.cloudfront.net/Resources/files/TemporalAA_small-59732822.pdf)

[Moving Frostbite to Physically Based Rendering 3.0](https://seblagarde.files.wordpress.com/2015/07/course_notes_moving_frostbite_to_pbr_v32.pdf)

### 附录1

![section00](D:\Code\p0\DRenderer\log\pic\section00.png)