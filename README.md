粒子系统项目说明文档

项目类型：基于OpenGL Compute Shader的GPU粒子系统

一、技术栈

开发库：
  - GLFW3          - 窗口管理和输入处理
  - GLM            - 数学运算库
  - gl3w           - OpenGL函数加载器

渲染技术：
  - Compute Shader - GPU计算着色器，用于粒子物理模拟
  - Shader Storage Buffer Objects (SSBO) - 粒子数据存储
  - Uniform Buffer Objects (UBO) - 着色器参数传递
  - Frame Buffer Objects (FBO) - 离屏渲染
  - Bloom后处理效果 - 光晕效果

着色器文件：
  - basePass.verrt / basePass.frag
  - particlePass.cs
  - bloomExtractVS.glsl / bloomExtractFS.glsl - Bloom高亮提取
  - bloomDownsampleFS.glsl - Bloom降采样
  - bloomUpsampleBilateralFS.glsl - Bloom双边滤波上采样
  - bloomCombineFS.glsl - Bloom合成

二、功能特性

1. GPU加速粒子系统
   - 使用Compute Shader在GPU上并行计算粒子位置和速度
   - 支持海量粒子
   - 基于分形布朗运动(fBm)的粒子运动

2. 交互控制
   - 吸引子效果
   - 粒子呼吸动画效果
   - 初始为爱心形状，随后开始分形布朗运动

3. 相机控制
   - 球坐标系相机系统
   - 鼠标拖动旋转和平移
   - 滚轮缩放

4. 视觉特效
   - Bloom光晕后处理
   - 多重降采样和上采样
   - 双边滤波边缘保持

三、操作说明

键盘控制：
  SPACE     - 切换动画播放/暂停
  A         - 切换吸引子效果开关
  R         - 重置粒子系统
  ESC       - 退出程序

鼠标控制：
  左键拖动  - 旋转相机视角
  右键拖动  - 平移相机
  鼠标滚轮  - 缩放（拉近/拉远）

