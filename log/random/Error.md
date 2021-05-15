如果遇到 sb 错误先到这里来看

* normalize
* 向量和矩阵的相乘顺序
* 行主导还是列主导
* tbn 的扩展不要有贡献, float4(t, 0.0)
* 金属太黑了感觉
* 不对, 和贵物一样
* 由于环境光没有, 所以 diffuse 项不能搞 1.0 - metallic, 要不就 gg 了
* f0 显然是反射越强的地方越大, 所以弄成 metallic * diffuse 是 ok 的
* 明天测下小球, 弄下 ibl 还有 taa 还有 msaa
* gamma correction: 没感觉到作用, 需要一个 ref
* 着色, 采样都存在一定问题
* 唉