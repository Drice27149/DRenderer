## 背景

Cluster Shading 是一项用于光照剔除 (light culling) 的技术。

在对像素进行着色时, 每一个光源都需要计算一次光照,  然而场景中一个光源影响的范围往往并不大, 只有一部分的像素需要考虑这个光源, 剩余像素的计算都被浪费掉了。因此, 在着色之前进行适当的光照剔除, 可以节省光照计算, 提高渲染效率。

## 实现方案

算法的思想并不复杂: 把相机视锥体划分成若干 3D 网格, 每个网格持有一个光源列表。对场景中的每个光源, 把它加入到它能够照亮的网格的光源列表中。在实际进行像素着色时, 先找到像素属于的网格, 然后对考虑网格光源列表里面的光源, 进行光照计算。

然而实现起来需要花些力气: 视锥体划分出来的网格并不是标准的立方体, 而一个光源能够照亮的区域在几何上也有不同的表示 (球状, 锥状) 等, 实现网格和光源的求交会涉及复杂几何体的求交计算。

[DOOM2016](https://www.slideshare.net/TiagoAlexSousa/siggraph2016-the-devil-is-in-the-details-idtech-666) 中使用了一种巧妙的方法: 在裁剪空间中进行光源和网格的求交。在裁剪空间中, 3D网格经过透视投影变换后已经是AABB盒了, 可以方便地与光源几何体进行求交。

[Detroit: Become Human](https://www.gdcvault.com/play/1025420/Cluster-Forward-Rendering-and-Anti) 采用的是基于 GPU 的剔除方案, 在 compute shader 中完成网格和光源的求交以及光源列表的构建, 可惜 talk 里面并未涉及太多实现细节。

[Kevin Örtegren 的论文](https://www.diva-portal.org/smash/get/diva2:839812/FULLTEXT02.pdf) 中详细描述了基于 GPU 的 Cluster Shading 的实现步骤, 这个实现方法完全避开了复杂几何体的求交, 并且附带基于 DirectX12 的实现。

我实现了基于 GPU 的方案, 因此下面着重讲讲这个方案的实现步骤。

## 实现步骤

* 光源的几何表示

* Shell Pass: 计算光源覆盖的最小和最大深度

* Fill Pass: 构建光源列表

## 性能分析

@TODO















