看到了[国外的图形学入门路径](http://graphicscodex.com/projects/projects/?utm_source=com.tencent.tim&utm_medium=social&utm_oi=613429687314157568)， 又看到了里面的 GPU 加速 ray-tracing 部分, 决定把光子映射干掉了

决定自己造一个 GPU 加速, 基于 compute shader 的路径追踪器 (不使用任何 RTX API, 不使用 cuda)。

API 上考虑, DX12 是首选, 如果出现时间不够等意外在考虑 OpenGL。

难点主要如下:

* 没有断点, printf 的情况下的 debug

* 场景数据的存取在 cs 中的实现
* 递归在 cs 中的实现
* 性能瓶颈的排查
* 可能的加速方案 (扩展并行度, 不是像素级别并行?)

主要分成三步走:

* 纯 CPU, 多线程的路径追踪器
* DX12 API 入门
* GPU based path-tracer

预计本月走完 OpenGL renderer, 本周走完 unity shader 入门精要, 该路径追踪器排到下个月, 采用 1个月(CPU) + 2个月(GPU实现) , 三个月后完成。

步子太大会扯到蛋, 目前还是 focus on 光栅化渲染器和 unity shader 入门精要





