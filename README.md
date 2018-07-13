AMCMCPPM With Rhino Support
========
支持渲染Rhino场景的光线追踪的简单实现。

Introduction
--------
一个只能实现最基本功能的光子映射装置。
- 最基本的光子映射效果。
- 支持网格与NURBS曲面求交。
- 支持读取Rhino（使用openNURBS）场景中的相机、光源、材质、网格和简单曲面。
- 最基本的BVH加速以及曲面分割加速。
- OpenMP加速。

Tips & TODO
--------
- 把算法搬到GPU上。
- 对高光和混合折射-反射的支持。
- 更漂亮的构图
- **加各种**~~骗分~~**特效**（体积光、景深、超采样、法线贴图之类往上摞）。
- **早写早出图**。

Dependencies
--------
- openNURBS （只测试了Windows）
- libpng

Acknowledgement
--------
[AMCMCPPM](http://www.ci.i.u-tokyo.ac.jp/~hachisuka/amcmcppm.pdf) 原始论文

[NURBS Flattening](https://www.cs.utah.edu/~shirley/papers/raynurbs.pdf) NURBS切分相关