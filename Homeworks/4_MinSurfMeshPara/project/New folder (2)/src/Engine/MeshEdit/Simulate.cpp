#include <Engine/MeshEdit/Simulate.h>

#include <math.h>
#include <Eigen/Sparse>

using namespace Ubpa;

using namespace std;
using namespace Eigen;


void Simulate::Clear() {
	this->positions.clear();
	this->velocity.clear();
}

bool Simulate::Init() {
	//Clear();
	g = -9.8;
	stiff = 100000;
	//g = 0;
	isfast = false;
	x.resize(3 * positions.size());
	y_.resize(3 * positions.size());
	xx.resize(3 * positions.size());
	iter = 3;
	SetFix();
	
	this->velocity.resize(positions.size());
	for (int i = 0; i < positions.size(); i++)
	{
		for (int j = 0; j < 3; j++)
		{
			this->velocity[i][j] = 0;
			x[3 * i + j] = positions[i][j];
			y_[3 * i + j] = positions[i][j];
			xx[3 * i + j] = positions[i][j];
		}
	}
	
	mass.resize(3 * positions.size());
	M.resize(3 * positions.size(), 3 * positions.size());
	M.fill(0);
	
	force_ext.resize(3 * positions.size());
	force_int.resize(3 * positions.size());
	for (int i = 0; i < 3 * positions.size(); i++)
	{
		force_ext[i] = 0.0;
		force_int[i] = 0.0;
		mass[i] = 1;
		M(i, i) = mass[i];
	}

	//gx_m.resize(3 * positions.size(), 1);
	
	for (int i = 0; i < edgelist.size() / 2; i++)
	{
		size_t index1 = edgelist[2 * i];
		size_t index2 = edgelist[2 * i + 1];

		pointf3 v1 = positions[index1];
		pointf3 v2 = positions[index2];

		l.push_back((v1 - v2).norm());
		//l.push_back(0.5);
	}

	SetInitG();
	GetSet();

	CacK();

	CacA();
	
	return true;
}


void Simulate::SetFast()
{
	isfast = true;
}

bool Simulate::Run() {
	SimulateOnce();

	// half-edge structure -> triangle mesh

	return true;
}

void Ubpa::Simulate::SetLeftFix()
{
	//固定网格x坐标最小点
	fixed_id.clear();
	double x = 100000;
	for (int i = 0; i < positions.size(); i++)
	{
		if (positions[i][0] < x)
		{
			x = positions[i][0];
		}
	}

	for (int i = 0; i < positions.size(); i++)
	{
		if (abs(positions[i][0] - x) < 1e-5)
		{
			fixed_id.push_back(i);
		}
	}

	Init();
}

void Simulate::SimulateOnce() {
	// TODO
	//cout << "WARNING::Simulate::SimulateOnce:" << endl;
//		<< "\t" << "not implemented" << endl;
	//stiff = 50;
	//stiff = 100000;
//SetStiff(100000);

	if (!isfast)
	{
		CacX();
		//void UpdateX();
		CacV();
		UpdatePos();
	}
	else
	{
		std::vector<double> x_=x;
		for (int i = 0; i < x.size(); i++)
			y_ [i]= 2 * x[i]  - xx[i];
		x = y_;
		for (int i = 0; i < iter; i++)
		{
			Local_CacD();
			Global_CacX();
		}
		xx = x_;
		UpdatePos();
	}
	
}

void Simulate::CacForce()
{
	force_int.clear();
	force_int.resize(3 * positions.size());
	for (int i = 0; i < edgelist.size() / 2; i++)
	{
		size_t index1 = edgelist[2 * i];
		size_t index2 = edgelist[2 * i + 1];

		//pointf3 v1 = positions[index1];
		//pointf3 v2 = positions[index2];
		pointf3 v1 = pointf3(x[3 * index1 + 0], x[3 * index1 + 1], x[3 * index1 + 2]);
		pointf3 v2 = pointf3(x[3 * index2 + 0], x[3 * index2 + 1], x[3 * index2 + 2]);
		vecf3 r = v1 - v2;

		for (int j = 0; j < 3; j++)
		{
			force_int[3 * index1 + j] += -stiff * (r.norm() - l[i]) * r[j] / r.norm();
			force_int[3 * index2 + j] += stiff * (r.norm() - l[i]) * r[j] / r.norm();
		}

	}
}

void Simulate::SetInitG()
{
	for (int i = 0; i < positions.size(); i++)
		force_ext[i * 3 + 1] = g * mass[i];
}



void Simulate::GetSet()
{
	//fix.insert(10);
	//fix.insert(120);
	//fix.insert(440);
	//fix.insert(20);
	//fix.insert(1*positions.size() / 4);
	//fix.insert(2 * positions.size() / 4);
	fix = std::set<int>(fixed_id.begin(), fixed_id.end());
}

void Simulate::CacK()
{
	K.resize(x.size() - 3 * fix.size(), x.size());
	K.fill(0);

	for (int i = 0, j = 0; i < x.size(); i++)
	{
		if (fix.find(i / 3) == fix.end())
		{
			K(j, i) = 1;
			//std::cout << j << " " << i << endl;
			j++;
		}
	}

	Eigen::MatrixXd xt;
	xt.resize((x.size()), 1);
	b.resize(x.size());

	for (int i = 0; i < x.size(); i++)
		xt(i, 0) = x[i];
	Eigen::MatrixXd t = K.transpose() * K * xt;

	for (int i = 0; i < x.size(); i++)
		b[i] = x[i] - t(i, 0);
}

void Simulate::CacX()
{
	std::vector<double> y(x.size());
	for (int i = 0; i < x.size(); i++)
		y[i] = x[i] + h * velocity[i / 3][i % 3] + h * h / mass[i] * force_ext[i];

	xk = y;
	int i = 0;
	do
	{
		CacForce();
		CacGX();
		CacDiff();


		//Eigen::MatrixXd inverG = diff_.inverse();

		CacGxM();

		Eigen::MatrixXd t = inverG * gx_m;

		xk_1.clear();
		xk_1.resize(gx.size());
		for (int i = 0; i < gx.size(); i++)
		{
			xk_1[i] = xk[i] - t(i, 0);
			//cout << t(i, 0) << endl;
		}
		i++;
		//cout << i++ << endl;
		UpdateX();
		xk = x;
	} while (!isconv() && i <= 10);//sparse 50


}

void Simulate::CacGxM()
{
	gx_m.resize(gx.size(), 1);
	for (int i = 0; i < gx.size(); i++)
		gx_m(i, 0) = gx[i];
}

bool Simulate::isconv()
{
	bool flag = true;
	double delta = 0.01;
	std::vector<double> zero(gx.size(), delta);

	for (int i = 0; i < gx.size() && flag; i++)
	{
		if (abs(gx[i]) > zero[i])
			flag = false;
	}
	return flag;
}

void Simulate::SetFix()
{
	//fixed_id.push_back(20);
	//fixed_id.push_back(440);
	//fixed_id.push_back(0);
	fixed_id.push_back(10);
	fixed_id.push_back(120);
}

void Simulate::CacGX()
{
	std::vector<double> y(xk.size());
	gx.resize(xk.size());


	for (int i = 0; i < xk.size(); i++)
	{
		y[i] = xk[i] + h * velocity[i / 3][i % 3] + h * h / mass[i] * force_ext[i];
	}


	for (int i = 0; i < xk.size(); i++)
	{
		gx[i] = mass[i] * (xk[i] - y[i]) - h * h * force_int[i];
	}

	Eigen::MatrixXd t(gx.size(), 1);
	for (int i = 0; i < gx.size(); i++)
		t(i, 0) = gx[i];

	t = K * t;

	gx.clear();
	gx.resize(t.rows());
	for (int i = 0; i < gx.size(); i++)
		gx[i] = t(i, 0);



	Eigen::MatrixXd xt;
	xt.resize((xk.size()), 1);

	for (int i = 0; i < xk.size(); i++)
		xt(i, 0) = xk[i];

	t = K * xt;
	xk.clear();
	xk.resize(t.rows());
	for (int i = 0; i < xk.size(); i++)
		xk[i] = t(i, 0);
}

void Simulate::CacDiff()
{
	std::vector<Eigen::Triplet<double> > triple;
	//Eigen::MatrixXd I = Eigen::DiagonalMatrix<double,3,3>::DiagonalMatrix(2);
	Eigen::MatrixXd I = MatrixXd::Identity(3, 3);
	for (int i = 0; i < edgelist.size() / 2; i++)
	{
		size_t index1 = edgelist[2 * i];
		size_t index2 = edgelist[2 * i + 1];

		pointf3 v1 = pointf3(x[3 * index1 + 0], x[3 * index1 + 1], x[3 * index1 + 2]);
		pointf3 v2 = pointf3(x[3 * index2 + 0], x[3 * index2 + 1], x[3 * index2 + 2]);
		vecf3 r = v1 - v2;
		MatrixXd t(3, 1);
		t(0, 0) = r[0]; t(1, 0) = r[1]; t(2, 0) = r[2];

		Eigen::MatrixXd dif;
		dif.resize(3, 3);
		dif = stiff * (l[i] / r.norm() - 1) * I - stiff * l[i]
			/ ((r.norm()) * (r.norm()) * (r.norm())) * t * t.transpose();
		//cout << index1 << " " << index2 << endl;
		//cout << dif << endl;
		for (int j = 0; j < 3; j++)
			for (int k = 0; k < 3; k++)
			{
				triple.push_back(Eigen::Triplet<double>(3 * index1 + j, 3 * index1 + k, -h * h * dif(j, k)));
				//triple.push_back(Eigen::Triplet<double>(3 * index1 + j, 3 * index2 + k, h*h*dif(j, k)));
				//triple.push_back(Eigen::Triplet<double>(3 * index2 + k, 3 * index1 + j, h*h*dif(j, k)));
				triple.push_back(Eigen::Triplet<double>(3 * index2 + k, 3 * index2 + j, -h * h * dif(j, k)));
			}
	}

	for (int i = 0; i < x.size(); i++)
	{
		triple.push_back(Eigen::Triplet<double>(i, i, mass[i]));
	}

	Eigen::MatrixXd diff_;
	Eigen::SparseLU<Eigen::SparseMatrix<double>> LU_;

	diff.setZero();
	diff.resize(x.size(), x.size());
	diff.setFromTriplets(triple.begin(), triple.end());

	diff_ = K * diff * K.transpose();
	//cout << diff_ << endl;
	//cout << K << endl;

	diff = diff_.sparseView();

	I = MatrixXd::Identity(gx.size(), gx.size());
	LU_.analyzePattern(diff);
	LU_.factorize(diff);
	//LU_.compute(diff);
	inverG = LU_.solve(I);
	//cout << inverG << endl;
}

void Simulate::UpdateX()
{
	Eigen::MatrixXd xt;
	xt.resize((xk_1.size()), 1);

	for (int i = 0; i < xk_1.size(); i++)
		xt(i, 0) = xk_1[i];

	Eigen::MatrixXd t = K.transpose() * xt;

	for (int i = 0; i < x.size(); i++)
		x[i] = t(i, 0) + b[i];

}

void Simulate::UpdatePos()
{
	for (int i = 0; i < x.size(); i++)
	{
		positions[i / 3][i % 3] = x[i];
	}
}

void Simulate::CacV()
{
	for (int i = 0; i < x.size(); i++)
	{
		velocity[i / 3][i % 3] = (x[i] - positions[i / 3][i % 3]) / h;
	}
}


void Simulate::CacL()
{
	L = MatrixXd::Zero(positions.size() * 3, positions.size() * 3);

	MatrixXd t = MatrixXd::Zero(positions.size(), positions.size());
	//cout << t << endl;
	for (int i = 0; i < edgelist.size() / 2; i++)
	{
		size_t index1 = edgelist[2 * i];
		size_t index2 = edgelist[2 * i + 1];

		std::vector<double> Ai(positions.size(),0);
		Ai[index1] = 1;
		Ai[index2] = -1;
		
		Eigen::MatrixXd At;
		At.resize((Ai.size()), 1);

		for (int j = 0; j < Ai.size(); j++)
			At(j, 0) = Ai[j];
		//cout << At << endl<<endl;
		t += stiff * At * At.transpose();
	}
	//cout << t << endl;
	//cout << M << endl;
	Matrix3d I3 = Matrix3d::Identity();
	for (int i = 0; i < positions.size(); i++)
	{
		for (int j = 0; j < positions.size(); j++)
			L.block(i * 3, j * 3, 3, 3) = t(i, j) * I3;
	}
}

void Simulate::CacJ()
{
	J = MatrixXd::Zero(positions.size() * 3, edgelist.size() / 2 * 3);

	MatrixXd t = MatrixXd::Zero(positions.size(), edgelist.size() / 2);

	for (int i = 0; i < edgelist.size() / 2; i++)
	{
		size_t index1 = edgelist[2 * i];
		size_t index2 = edgelist[2 * i + 1];

		std::vector<double> Ai(positions.size(), 0);
		Ai[index1] = 1;
		Ai[index2] = -1;

		std::vector<double> Si(edgelist.size() / 2,0);
		Si[i] = 1;

		Eigen::MatrixXd At;
		At.resize((Ai.size()), 1);

		for (int j = 0; j < Ai.size(); j++)
			At(j, 0) = Ai[j];

		Eigen::MatrixXd St;
		St.resize((Si.size()), 1);

		for (int j = 0; j < Si.size(); j++)
			St(j, 0) = Si[j];
		//cout << stiff * At * St.transpose() << endl;
		t += stiff * At * St.transpose();
	}
	//cout << t << endl;
	Matrix3d I3 = Matrix3d::Identity();
	for (int i = 0; i < positions.size(); i++)
	{
		for (int j = 0; j < edgelist.size() / 2; j++)
			J.block(i * 3, j * 3, 3, 3) = t(i, j) * I3;
	}
}

void Simulate::CacA()
{
	CacL();
	CacJ();
	MatrixXd A_;
	//cout << L << endl;
	//cout << J << endl;
	A_.resize(K.rows(), K.rows());
	A_ = K * (M + h * h * L) * K.transpose();
	A = A_.sparseView();
	LLT_.compute(A);
}

void Simulate::Global_CacX()
{
	Eigen::MatrixXd dt;
	dt.resize((d.size()), 1);

	for (int j = 0; j < d.size(); j++)
		dt(j, 0) = d[j];

	Eigen::MatrixXd yt;
	yt.resize((y_.size()), 1);

	for (int j = 0; j < y_.size(); j++)
		yt(j, 0) = y_[j];

	Eigen::MatrixXd ft;
	ft.resize((force_ext.size()), 1);

	for (int j = 0; j < force_ext.size(); j++)
		ft(j, 0) = force_ext[j];

	Eigen::MatrixXd bt;
	bt.resize((b.size()), 1);

	for (int j = 0; j < b.size(); j++)
		bt(j, 0) = b[j];

	VectorXd B = K * (h * h * J * dt + M * yt + h * h * ft - (M + h * h * L) * bt);
	//cout << J << endl;
	//cout << L << endl;
	VectorXd xf = LLT_.solve(B);
	//cout << xf << endl;
	xf = K.transpose() * xf + bt;

	for (int j = 0; j < x.size(); j++)
		x[j] = xf[j];
}

void Simulate::Local_CacD()
{
	d.resize(3 * edgelist.size() / 2);
	
	for (int i = 0; i < edgelist.size() / 2; i++)
	{
		size_t index1 = edgelist[2 * i];
		size_t index2 = edgelist[2 * i + 1];
		pointf3 v1 = pointf3(x[3 * index1 + 0], x[3 * index1 + 1], x[3 * index1 + 2]);
		pointf3 v2 = pointf3(x[3 * index2 + 0], x[3 * index2 + 1], x[3 * index2 + 2]);
		vecf3 r = v1 - v2;

		d[3 * i + 0] = l[i] * r[0] / r.norm();
		d[3 * i + 1] = l[i] * r[1] / r.norm();
		d[3 * i + 2] = l[i] * r[2] / r.norm();
	}
}
