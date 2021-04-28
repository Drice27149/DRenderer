* 主要参考 doom 2016

  * Pre-Z, 剔除 & cluster的准备工作
  * Forward Passes
    * 建立 cluster
    * 读纹理, 计算光照, 输出一层薄的 G-Buffer
  * Deffered Passes 
    * 反射, AO, 雾
  * 透明物体
  * 后处理

* 光源类型

  * 点光源, 锥形光源, 平行光, 区域光 (quad, disk, sphere), IBL (environment probe)
  * environment Probes 是 128x128 的固定分辨率

* 未接触过的 feature

  * SSR
  * Specular/Ambient Occlusion 