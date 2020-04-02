## Lab5 Arap &&Asap

PB17111585 张永停

### 一、实验内容

- 在给定的网格框架上完成作业，实现
  - ASAP (As-similar-as-possible) 参数化算法
  - ARAP (As-rigid-as-possible) 参数化算法
  - Hybrid 参数化方法
- 对各种参数化方法（包括作业4的Floater方法、ASAP/ARAP方法等）进行比较
- 继续学习和巩固三角网格的数据结构及编程
- 学习和实现矩阵的 SVD 分解
- 进一步巩固使用 Eigen 库求解大型稀疏线性方程组

### 二、算法描述

#### （一）Arap

- 总体思想:
  - Local:求解Lt
  - Global:求解Ut

- 将曲面的三角形全等映射到二维平面得到$x_t=(x_t^0,x_t^1,x_t^2)$

- Local:以CoTan算法得到的参数$u_t=(u_t^0,u_t^1,u_t^2)$作为Ut的初始输入，根据

  $$S_t=\sum_{i=0}^2 cot(\theta_t^i)(u_t^i-u_t^{i+1})(x_t^i-x_t^{i+1})^T$$来计算$S_t$，之后对$S_t$进行SVD分解，计算$L_t=UV^T$

- Global:解下列方程，即可得到Arap参数化后的坐标$u_1,...,u_N$

  $$\sum_{j\in N(i))}[cot(\theta_{ij})+cot(\theta_{ji})](u_i-u_j)=\sum_{j\in N(i))}[cot(\theta_{ij})L_{t(i,j)}+cot(\theta_{ji}L_{t(j,i)})](x_i-x_j)$$

  这里需要注意的是上式右端两处$x_i,x_j$坐标并不一样，一次是ij所属的半边，一次是ji所属的半边

#### （二）Asap

- 同样采用Local/Global的方法求解，仅在Lt的求解过程中与Arap不同
- 计算$L_t=\dfrac{\sigma_1+\sigma_2}{2}UV^T$，其中$\sigma_1,\sigma_2$是奇异值
- 这里需要注意，由于奇异值很小，故计算后Ut坐标很小，因而采用归一化的方法，将Ut坐标放大到[0,1]*[0,1]

#### （三）Hybrid

- 同样采用Local/Global的方法求解，仅在Lt的求解过程中与Arap不同
  - 通过$\dfrac{2\lambda(C_2^2+C_3^2)}{C_2^2}+(C_1-2\lambda)a-C_2=0$来求解$a$，这是一个一元三次方程，通过盛金公式来求解
  - 通过$b=(C_3/C_2)a$来求解b
  - $L_t=\begin{pmatrix} a & b \\ -b & a \end{pmatrix}$
  - 其中$C_1,C_2,C_3$见论文附录
- 同于Asap,需要对坐标放缩一下

### 三、代码框架

#### （一）Arap

继承自上次作业的Paramaterize

```c++
private:
		std::vector<Eigen::Vector2d> Xt_;			//全等映射后坐标
		std::vector<Eigen::Vector2d> Ut_;			//参数化坐标
		std::vector<Eigen::Matrix2d> Lt_;				
		std::vector<double> Cot_;					//每条半边对应角的cot

		Eigen::SparseMatrix<double> Cof_;			//稀疏方程组左端系数
		Eigen::MatrixX2d b_;						//稀疏方程组右端系数
		Eigen::MatrixX2d X_;						//方程组的解
		Eigen::SparseLU<Eigen::SparseMatrix<double>> LU_;

		std::map<THalfEdge<V, E, P>*, int> he_to_index;
													//将半边映射到一个index
		int pos_not_move;							//锚点
		Eigen::Vector2d pos_coor;					//锚点坐标
private:
		void ParaByArap();							//使用arap的方法，包括迭代

protected:
		void initUt();								//使用CotWeight初始化Ut
		void GetMap();								//得到映射
		void CacXtAndCot();							//计算Xt与Cot
		void CacLt();					
		void CacB();
		void CacCof();
		void UpdateUt();							//将方程组的解更新值Ut

		void SetPos();								//选择锚点

		void UpdateHeMesh();						//更新半边数据结构

		double CacCot(THalfEdge<V,E,P> *he);		//计算一条半边的cot
```

#### （二）Asap

```c++
protected:
		void Normlize();							//对点的坐标归一化
													//其余同Arap
```

#### （三）Hybrid

```c++
private:		
		std::vector<double> root;					//存放三次方程的解
protected:
		void ShengJin(double a, double b, double c, double d);
													//解三次方程
													//其余同Asap
```

### 四、实验结果

- Ball

  <figure class="fouth">
        <img src="b.png", width=250>
        <img src="arap_3.png", width=250>
        <img src="arap_5.png", width=250>
        <img src="ball_asap.png", width=250>
  </figure>

  <i>右上：arap 4次 左下 arap 6次 右下 asap</i>

- Beetle

  <figure class="half">
        <img src="beetle_1.png", width=250>
        <img src="beetle.png", width=250>
  </figure>

- Cow

  <figure class="half">
        <img src="c.png", width=250>
        <img src="cow_3_arap.png", width=250>
  </figure>

- Gargoyle

  <figure class="half">
        <img src="g.png", width=250>
        <img src="g_arap_3.png", width=250>
  </figure>



### 五、实验总结

- 本次实验耗时巨久，一是接口不熟练，二是犯一些特别低级的错误。。。比如认为两个指向同一地址的指针相同。。。（就是这个问题我debug了一整天。。。）
- 然后是Hybird的部分，我调到了最后一分钟，但结果还是不对，难道不是只改Lt嘛TAT，求解释