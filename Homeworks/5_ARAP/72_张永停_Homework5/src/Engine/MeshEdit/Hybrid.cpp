#include <Engine/MeshEdit/Hybird.h>

#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <math.h>
#include <map>

using namespace Ubpa;

using namespace std;

Hybird::Hybird(Ptr<TriMesh> triMesh) :Paramaterize(triMesh), heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

void Hybird::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}


bool Hybird::Init(Ptr<TriMesh> triMesh) {
	cout << (triMesh == nullptr) << endl;
	Clear();
	tex_ = false;
	lambda = 0;
	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::MinSurf::Init:\n"
			"\t""trimesh is invalid\n");
		return false;
	}

	//init Ut



	// init half-edge structure
	size_t nV = triMesh->GetPositions().size();
	vector<vector<size_t>> triangles;
	triangles.reserve(triMesh->GetTriangles().size());
	for (auto triangle : triMesh->GetTriangles())
		triangles.push_back({ triangle->idx[0], triangle->idx[1], triangle->idx[2] });
	heMesh->Reserve(nV);
	heMesh->Init(triangles);

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary()) {
		printf("ERROR::MinSurf::Init:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		heMesh->Clear();
		return false;
	}

	// triangle mesh's positions ->  half-edge structure's positions
	for (int i = 0; i < nV; i++) {
		auto v = heMesh->Vertices().at(i);
		v->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
	}

	this->triMesh = triMesh;

	return true;

}

void Hybird::SetPos()
{
	for (auto v : heMesh->Vertices())
	{
		if (v->IsBoundary())
		{
			pos_not_move = heMesh->Index(v);
			return;
		}
	}

	pos_not_move = 0;
}

void Hybird::initUt()
{
	auto cotp = Paramaterize::New(triMesh);
	cotp->SetWMethod(2);
	Ptr < HEMesh < V >> init_hemesh = cotp->ReturnPos();

	Ut_.clear();

	Ut_.resize(heMesh->NumHalfEdges());

	int i = -1;

	for (auto p : init_hemesh->Polygons())
	{
		i++;
		auto p2 = heMesh->Polygons()[i];

		if (p != nullptr)
		{
			auto edges = p->BoundaryHEs();
			auto edges2 = p2->BoundaryHEs();

			V* v1 = edges[0]->Origin();
			V* v2 = edges[0]->End();
			V* v3 = edges[0]->Next()->End();

			Ut_[he_to_index[edges2[0]]] = Eigen::Vector2d(v1->pos[0], v1->pos[1]);

			Ut_[he_to_index[edges2[0]->Next()]] = Eigen::Vector2d(v2->pos[0], v2->pos[1]);
			//cout << he_to_index[edges2[0]->Next()]<< endl;
			Ut_[he_to_index[edges2[0]->Next()->Next()]] = Eigen::Vector2d(v3->pos[0], v3->pos[1]);
		}
	}
	V* p = init_hemesh->Vertices()[pos_not_move];
	pos_coor = Eigen::Vector2d(p->pos[0], p->pos[1]);

}

bool Hybird::Run()
{
	//TODO
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}


	ParaByHybird();


	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf3> positions;
	vector<pointf2> texcoords;

	vector<unsigned> indice;
	positions.reserve(nV);
	texcoords.reserve(nV);
	indice.reserve(3 * nF);
	for (auto v : heMesh->Vertices())
	{
		positions.push_back(v->pos.cast_to<pointf3>());
		//cout << v->pos << endl;
		texcoords.push_back(v->pos.cast_to<pointf2>());
	}

	for (auto f : heMesh->Polygons()) { // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}
	//if (tex_)
		//triMesh->Update(texcoords);
	//else
	triMesh->Update(positions);


	return true;
	//todo
}

void Hybird::SetTex(int m)
{
	tex_ = true;
}

void Hybird::ParaByHybird()
{
	GetMap();
	CacXtAndCot();
	SetPos();
	initUt();

	for (int i = 0; i <= 0; i++)
	{
		CacLt();
		CacB();
		CacCof();

		LU_.compute(Cof_);
		X_ = LU_.solve(b_);

		Normlize();
		UpdateUt();
		UpdateHeMesh();
	}

}

void Hybird::CacXtAndCot()
{
	int edgenum = heMesh->NumHalfEdges();

	Xt_.resize(edgenum);
	Cot_.resize(edgenum);

	for (auto p : heMesh->Polygons())
	{
		if (p != nullptr)
		{
			auto edges = p->BoundaryHEs();
			for (auto e : edges)
			{
				double cot = CacCot(e);
				//cout << cot << endl;
				Cot_[he_to_index[e]] = cot;
			}

			V* v1 = edges[0]->Origin();
			V* v2 = edges[0]->End();
			V* v3 = edges[0]->Next()->End();

			//cout << he_to_index[edges[0]] << " " << he_to_index[edges[1]] << " " << he_to_index[edges[2]] << endl; 
			//if (he_to_index[edges[0]] == 1 || he_to_index[edges[1]] == 1 || he_to_index[edges[1]] == 1)
			//	int i = 1;
			Xt_[he_to_index[edges[0]]] = Eigen::Vector2d(0, 0);
			Xt_[he_to_index[edges[1]]] = Eigen::Vector2d((v1->pos - v2->pos).norm(), 0);
			Xt_[he_to_index[edges[2]]] =
				Eigen::Vector2d((v2->pos - v1->pos).dot(v3->pos - v1->pos),
				(v2->pos - v1->pos).cross(v3->pos - v1->pos).norm() / ((v1->pos - v2->pos).norm()));
			//cout << v1->pos << endl;
			//cout << v2->pos << endl;
			//cout << v3->pos << endl;
			//cout << (v2->pos - v1->pos).dot(v3->pos - v1->pos)<<"dot" << endl;
			//cout << (v2->pos - v1->pos).cross(v3->pos - v1->pos).norm() / (v1->pos - v2->pos).norm() << endl;
			//break;
		}
	}
}

void Hybird::GetMap()
{
	int i = 0;
	for (auto e : heMesh->HalfEdges())
	{
		he_to_index.insert(std::pair<THalfEdge<V, E, P>*, int>(e, i));
		i++;
		//cout << i << endl;
	}
}

double Hybird::CacCot(THalfEdge<V, E, P> * he)
{
	V* v1 = he->Origin();
	V* v2 = he->End();
	V* v3 = he->Next()->End();

	double cos = vecf3::cos_theta((v1->pos - v3->pos), (v2->pos - v3->pos));
	double sin = sqrt(1 - cos * cos);

	return cos / sin;
}

void Hybird::CacLt()
{
	Lt_.clear();

	Lt_.resize(heMesh->NumPolygons());

	for (auto p : heMesh->Polygons())
	{
		if (p != nullptr)
		{
			auto edges = p->BoundaryHEs();
			double C1=0.0, C2=0.0, C3=0.0;
			for (int i = 0; i <= 2; i++)
			{
				double wi = Cot_[he_to_index[edges[i]]];
				Eigen::Vector2d  ui, xi;
				ui = Ut_[he_to_index[edges[i]]] - Ut_[he_to_index[edges[i]->Next()]];
				xi = Xt_[he_to_index[edges[i]]] - Xt_[he_to_index[edges[i]->Next()]];

				C1 += wi * (xi.norm()* xi.norm());
				C2 += wi * (ui.dot(xi));
				C3 += wi * (ui(0) * xi(1) - ui(1) * xi(0));
			}

			double a = 0.0;
			double b;
			if (lambda != 0)
			{
				ShengJin(2 * lambda * (C2 * C2 + C3 * C3) / (C2 * C2), 0, C1 - 2 * lambda, -C2);
				a = root[0];
				b = C3 / C2 * a;
			}
			else
			{
				a = C2 / C1;
				b = C3 / C1;
			}
				
			Lt_[heMesh->Index(p)] << a, b, -b, a;
			//cout << Lt_[heMesh->Index(p)];
		}
	}
}

void Hybird::CacB()
{
	b_ = Eigen::MatrixX2d::Zero(heMesh->NumVertices(), 2);

	b_(pos_not_move, 0) = pos_coor[0];
	b_(pos_not_move, 1) = pos_coor[1];


	int i = -1;

	for (auto v : heMesh->Vertices())
	{
		i++;

		int k = heMesh->Index(v);

		if (k == pos_not_move)
			continue;

		Eigen::Vector2d b;
		b = Eigen::Vector2d::Zero();
		for (auto adj : v->AdjVertices())
		{
			auto he1 = v->HalfEdgeTo(adj);
			if (he1 != nullptr)
			{
				auto he1_next = he1->Next();
				auto p1 = he1->Polygon();
				if (p1 != nullptr)
					b += (Cot_[he_to_index[he1]] * Lt_[heMesh->Index(p1)]) *
					(Xt_[he_to_index[he1]] - Xt_[he_to_index[he1_next]]);
			}

			auto he2 = adj->HalfEdgeTo(v);
			if (he2 != nullptr)
			{
				auto he2_next = he2->Next();
				auto p2 = he2->Polygon();
				if (p2 != nullptr)
					b -= (Cot_[he_to_index[he2]] * Lt_[heMesh->Index(p2)]) *
					(Xt_[he_to_index[he2]] - Xt_[he_to_index[he2_next]]);
			}



			//cout << "b" << Cot_[he_to_index[he1]] << endl;
			//cout << Lt_[heMesh->Index(p1)] << endl;
			//cout << Xt_[he_to_index[he1]] << endl;
			//cout << Xt_[he_to_index[he2]] << endl;
			//cout << he_to_index[he1] << " " << he_to_index[he2] << endl;

			//cout << "b" << Cot_[he_to_index[he2]] << endl;
			//cout << Lt_[heMesh->Index(p2)] << endl;
		}

		b_(k, 0) = b(0);
		b_(k, 1) = b(1);
	}
}


void Hybird::CacCof()
{
	Cof_.resize(heMesh->NumVertices(), heMesh->NumVertices());

	double w, sum_w;

	int index = pos_not_move;
	Cof_.insert(index, index) = 1.0;

	int i = -1;
	for (auto v : heMesh->Vertices())
	{
		i++;
		int k = heMesh->Index(v);
		if (k == pos_not_move)
			continue;

		sum_w = 0;

		for (auto adj : v->AdjVertices())
		{
			w = 0;
			int j = heMesh->Index(adj);

			auto he1 = v->HalfEdgeTo(adj);
			auto he2 = adj->HalfEdgeTo(v);

			auto p1 = he1->Polygon();
			auto p2 = he2->Polygon();

			if (p1 != nullptr)
				w += Cot_[he_to_index[he1]];

			if (p2 != nullptr)
				w += Cot_[he_to_index[he2]];


			sum_w += w;
			Cof_.insert(k, j) = -w;
		}
		Cof_.insert(k, k) = sum_w;
	}
}

void Hybird::Normlize()
{
	Eigen::Vector2d mincoor, maxcoor;

	mincoor.fill(100000);
	maxcoor.fill(0);

	for (int i = 0; i < heMesh->NumVertices(); i++)
	{
		if ((X_(i, 0)) < mincoor(0))
			mincoor(0) = X_(i, 0);

		if ((X_(i, 0)) > maxcoor(0))
			maxcoor(0) = X_(i, 0);

		if (X_(i, 1) < mincoor(1))
			mincoor(1) = X_(i, 0);

		if (X_(i, 1) > maxcoor(1))
			maxcoor(1) =X_(i, 1);
	}

	for (int i = 0; i < heMesh->NumVertices(); i++)
	{
		double w = maxcoor(0) - mincoor(0) > maxcoor(1) - mincoor(1) ? maxcoor(0) - mincoor(0) : maxcoor(1) - mincoor(1);
		X_(i, 0) = X_(i, 0) / w;
		X_(i, 1) = X_(i, 1) / w;
	}

}

void Hybird::UpdateUt()
{
	for (auto p : heMesh->Polygons())
	{
		if (p != nullptr)
		{
			auto edges = p->BoundaryHEs();

			V* v1 = edges[0]->Origin();
			V* v2 = edges[0]->End();
			V* v3 = edges[0]->Next()->End();

			int i1 = heMesh->Index(v1);
			int i2 = heMesh->Index(v2);
			int i3 = heMesh->Index(v3);

			Ut_[he_to_index[edges[0]]] = Eigen::Vector2d(X_(i1, 0), X_(i1, 1));
			Ut_[he_to_index[edges[1]]] = Eigen::Vector2d(X_(i2, 0), X_(i2, 1));
			Ut_[he_to_index[edges[2]]] = Eigen::Vector2d(X_(i3, 0), X_(i3, 1));
		}
	}
}

void Hybird::UpdateHeMesh()
{
	for (auto v : heMesh->Vertices())
	{
		int i = heMesh->Index(v);
		v->pos = vecf3({ X_(i, 0), X_(i, 1), 0 });
		//cout << i<<"  "<<X_(i, 0) << endl;
	}
}

void Hybird::ShengJin(double a, double b, double c, double d)
{
	double A = b * b - 3 * a * c;
	double B = b * c - 9 * a * d;
	double C = c * c - 3 * b * d;
	double f = B * B - 4 * A * C;

	double i_value;
	double Y1, Y2;
	root.clear();

	if (fabs(A) < 1e-6 && fabs(B) < 1e-6)//公式1
	{
		root.push_back(-b / (3 * a));
		root.push_back(-b / (3 * a));
		root.push_back(-b / (3 * a));
	}

	else if (fabs(f) < 1e-6)   //公式3
	{
		double K = B / A;
		root.push_back(-b / a + K);
		root.push_back(-K / 2);
		root.push_back(-K / 2);
	}

	else if (f > 1e-6)      //公式2
	{
		Y1 = A * b + 3 * a * (-B + sqrt(f)) / 2;
		Y2 = A * b + 3 * a * (-B - sqrt(f)) / 2;
		double Y1_value = (Y1 / fabs(Y1)) * pow((double)fabs(Y1), 1.0 / 3);
		double Y2_value = (Y2 / fabs(Y2)) * pow((double)fabs(Y2), 1.0 / 3);
		root.push_back((-b - Y1_value - Y2_value) / (3 * a));

		i_value = sqrt(3.0) / 2 * (Y1_value - Y2_value) / (3 * a);

		if (fabs(i_value) < 1e-1)
		{
			root.push_back((-b + 0.5 * (Y1_value + Y2_value)) / (3 * a));
		}
	}

	else if (f < -1e-6)   //公式4
	{
		double T = (2 * A * b - 3 * a * B) / (2 * A * sqrt(A));
		double S = acos(T);
		root.push_back((-b - 2 * sqrt(A) * cos(S / 3)) / (3 * a));
		root.push_back((-b + sqrt(A) * (cos(S / 3) + sqrt(3.0) * sin(S / 3))) / (3 * a));
		root.push_back((-b + sqrt(A) * (cos(S / 3) - sqrt(3.0) * sin(S / 3))) / (3 * a));
	}
}