## Lab7 基于 Taichi 的仿真

by PB17111585 张永停

### 一、实验内容

- 构建不同的模拟场景
  - 实现gui添加在画布任意位置添加椭圆/长方形物体，或者粒子发射源
  - 实现gui添加物体时设定初始速度，种类(流体/雪球/果冻/弹性木块)，颜色
  - 实现gui下修改杨氏模量
  - 实现gui暂停模拟(可恢复，并且暂停过程中可以新加入物体)，停止模拟(不可恢复，即删除)
  - 实现gui暂停模拟（可恢复，并可以新添加物块），停止模拟
  - 视频见`gui`文件夹下
- 探究参数空间：通过调整不同的参数设置体会不同参数对仿真结果的影响
  - 对于雪块，更改杨氏模量，结果存于`改变参数`文件夹下(请使用potplayer打开)。杨氏模量是体现物体弹性的量，由视频可发现，E越小，雪块弹性越小，这符合物理事实
  - 对于雪块，果冻，水三个物体改变杨氏模量，结果见视频
  - 对于雪块，更改硬度，结果存于`改变参数`文件夹下
- 探究其他算法和模型：通过改变物理模型、改变模拟算法实现新的模拟结果
  - 使用gui可以实现任意碰撞模型，只需要更改初始位置，初始速度即可

### 二、算法描述

- 本次实验gui采用HW1 MiniDraw的框架，增加了模拟相关的按钮以及控制

  - 初始速度大小，物体材料选择

  - 选择是否为粒子发射源

    若当前粒子是发射源，则每次模拟，都添加一个与当前物体性质一样的粒子团

  - 杨氏模量更改

  - 开始模拟，暂停模拟，停止模拟

- 对于Hw1的`shape`类，增加`velocity`，`material`属性

### 三、实验结果

- 更改雪块的杨氏模量

  <figure class="third">
        <img src="img\snow_young_1e2.png", width=150>
        <img src="img\snow_young_1e3.png", width=150>
        <img src="img\snow_young_1e4.png", width=150>
  </figure>

  <i>左: E=1e2，中 E=1e3，右 E=1e4</i>

- 更改雪块的硬度

  <figure class="third">
        <img src="img\snow_hard_1.png", width=150>
        <img src="img\snow_hard_5.png", width=150>
        <img src="img\snow_young_1e4.png", width=150>
  </figure>

  <i>左: hardening=1，中 hardening=5，右 hardening=10</i>

- GUI界面

  <figure class="third">
        <img src="img\gui_0.jpg", width=150>
        <img src="img\gui_4.jpg", width=150>
        <img src="img\gui_3.jpg", width=150>
  </figure>

  <i>模拟一个粒子发射源(紫)，一块雪块(蓝)，一块木块，一块果冻，以及暂停后在粒子轨道上方添加雪块(绿)</i>

  <div align="left"><img src="img\gui_1.jpg" width="450"></div>
<i>GUI界面</i>

### 四、实验总结

- 本次实验非常有趣，玩taichi玩的不亦乐乎，甚至还装了python版本。最开始想使用3D版本的taichi，即论文mpm的框架，但在linux上折腾了好久最后还是失败了TAT
- 后来又跑去研究了一波Houdini，虽然最后没有渲染成功，但还是学了不少的hh
- 将taichi转到QT的最大的两个困难，一个是原来代码坐标范围是`0-1`，并且y轴从下到上递增，这些细节最开始没注意到，导致调试了一段时间；第二个困难是，原代码模拟的时候使用死循环，这样阻塞了ui界面，最开始想试着多线程，可惜我掌握不熟练，最后查到了`QApplication::processEvents();`，以可以终结死循环。