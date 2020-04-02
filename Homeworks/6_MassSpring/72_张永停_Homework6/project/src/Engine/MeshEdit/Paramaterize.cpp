#include <Engine/MeshEdit/Paramaterize.h>

#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#define MINI_NUMBER 0.000001

using namespace Ubpa;

using namespace std;

Paramaterize::Paramaterize(Ptr<TriMesh> triMesh) : heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
	//cout << "Paramaterize::Paramaterize:" << endl
	//	<< "\t" << "not implemented" << endl;
}

void Paramaterize::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
	//cout << "Paramaterize::Clear:" << endl
	//	<< "\t" << "not implemented" << endl;
}

bool Paramaterize::Init(Ptr<TriMesh> triMesh) {
	// TODO
	//cout << "Paramaterize::Init:" << endl
	//	<< "\t" << "not implemented" << endl;
	cout << (triMesh == nullptr )<< endl;
	Clear();
	tex_ = false;
	type_ = 1;
	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::MinSurf::Init:\n"
			"\t""trimesh is invalid\n");
		return false;
	}

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

	inside_count_ = 0;
	inside_points_.clear();
	map_between_inside_.clear();

	return true;
	
}

bool Paramaterize::Run() {
	// TODO
	//cout << "Paramaterize::Init:" << endl
	//	<< "\t" << "not implemented" << endl;
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	Para();

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
		texcoords.push_back(v->pos.cast_to<pointf2>());
	}
		
	for (auto f : heMesh->Polygons()) { // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}
	//if(tex_)
	//	triMesh->Update(texcoords);
	//else
	//	triMesh->Update(positions);
	
	return true;
}

void Paramaterize::SetTex(int m)
{
	type_ = m;
	tex_ = true;
}

void Paramaterize::SetWMethod(int m)
{
	type_ = m;
}

Ptr<HEMesh<Paramaterize::V>> Paramaterize::ReturnPos()
{
	Run();
	return heMesh;
}

void Paramaterize::Para()
{
	FindInside();
	SetBoundary();
	GetMap();

	if(type_==1)
		ParaU();
	else
		ParaTan();
	
	LU_.compute(L_);
	X = LU_.solve(delta_);

	UpdatePara();
}

void Paramaterize::ParaU()
{
	L_.resize(inside_count_, inside_count_);
	delta_ = Eigen::MatrixX3d::Zero(inside_count_, 3);

	vector<Eigen::Triplet<double> > coefficients;

	for (int i = 0; i < inside_count_; i++)
	{
		V* v = inside_points_[i];
		double w = 1.0/(v->Degree());
		//cout <<"d"<< v->Degree() << endl;
		coefficients.push_back(Eigen::Triplet<double>(i, i, v->AdjVertices().size()));

		for (auto adj : v->AdjVertices())
		{
			int j = heMesh->Index(adj);
			if (!adj->IsBoundary())
			{
				j = map_between_inside_[j];
				coefficients.push_back(Eigen::Triplet<double>(i, j, -1));
				//cout << i << endl;
			}
			else
			{
				delta_(i, 0) += adj->pos[0];
				//cout <<"delta"<<" "<<i<<" "<< delta_(i, 0) << endl;
				delta_(i, 1) += adj->pos[1];
				delta_(i, 2) += adj->pos[2];
			}
		}
	}
	L_.setFromTriplets(coefficients.begin(), coefficients.end());
}


void Paramaterize::ParaTan()
{
	L_.resize(inside_count_, inside_count_);
	delta_ = Eigen::MatrixX3d::Zero(inside_count_, 3);

	vector<Eigen::Triplet<double> > coefficients;

	
	for (int i = 0; i < inside_count_; i++)
	{
		double sum_weight = 0.0;
		
		V* v = inside_points_[i];
		
		int d = v->Degree();
		//cout << i <<" "<<d<< endl;
		for (int j = 0; j < d; j++)
		{
			V* adj = v->AdjVertices()[j];
			
			int k= heMesh->Index(adj);
			V* adj1 = v->AdjVertices()[(j - 1 + d) % d];
			V* adj2 = v->AdjVertices()[(j + 1) % d];
			if (i == 13)
				cout<< adj->pos - adj2->pos <<endl;
			double cos1 = abs(vecf3::cos_theta((v->pos - adj1->pos), (adj->pos - adj1->pos)));
			double cos2 = abs(vecf3::cos_theta((v->pos - adj2->pos), (adj->pos - adj2->pos)));
			double sin1 = sqrt(1 - cos1 * cos1);
			double sin2 = sqrt(1 - cos2 * cos2);
			double cot1, cot2;
			if (sin1 < MINI_NUMBER)
				cot1 = sqrt(1.0 / (cos1 * cos1) - 1.0);
			else
				cot1 = cos1 / sin1;
			if (sin2 < MINI_NUMBER)
				cot2 = sqrt(1.0 / (cos2 * cos2) - 1.0);
			else
				cot2 = cos2 / sin2;
			double w = cot1+cot2;

			sum_weight += w;

			if (!adj->IsBoundary())
			{
				k = map_between_inside_[k];
				coefficients.push_back(Eigen::Triplet<double>(i, k, -1*w));
			}
			else
			{
				delta_(i, 0) += w*adj->pos[0];
				delta_(i, 1) += w*adj->pos[1];
				delta_(i, 2) += w*adj->pos[2];
			}
			
		}
		
		coefficients.push_back(Eigen::Triplet<double>(i, i, sum_weight));
	}
	L_.setFromTriplets(coefficients.begin(), coefficients.end());
}

void  Paramaterize::FindInside()
{
	for (auto v : heMesh->Vertices())
		if (!v->IsBoundary())
			//boundary_points_.push_back(v);
		//else
			inside_points_.push_back(v);
	
	vector<HEMesh<V>::HE*> boundaries = heMesh->Boundaries()[0];
	for (auto he : boundaries) 
		boundary_points_.push_back(he->Origin());
	inside_count_ = inside_points_.size();
	boundary_count_= boundary_points_.size();
	return;
}

void  Paramaterize::SetBoundary()
{
	for (int i = 0; i < boundary_count_ / 4; i++)
	{
		V* v = boundary_points_[i];
		v->pos = vecf3({ i*1.0/ (boundary_count_ / 4),0.0,0.0 });
	}

	for (int i = boundary_count_ / 4; i < boundary_count_ / 2; i++)
	{
		V* v = boundary_points_[i];
		v->pos = vecf3({1.0, 1.0*(i - boundary_count_ / 4) / (boundary_count_ / 2-boundary_count_ / 4),0.0 });
	}

	for (int i = boundary_count_ / 2; i < 3*boundary_count_ / 4; i++)
	{
		V* v = boundary_points_[i];
		v->pos = vecf3({1.0*(3*boundary_count_ / 4-i)/ (3*boundary_count_ / 4- boundary_count_ / 2),1.0,0.0 });
	}

	for (int i = 3 * boundary_count_ / 4; i < boundary_count_ ; i++)
	{
		V* v = boundary_points_[i];
		v->pos = vecf3({ 0.0,( boundary_count_ - i) *1.0/ (boundary_count_ - 3 * boundary_count_ / 4),0.0 });
	}

}


void Paramaterize::GetMap()
{
	for (int i = 0; i < inside_count_; i++)
		map_between_inside_.insert({ heMesh->Index(inside_points_[i]), i });
	//cout << heMesh->Index(inside_points_[i]) << endl;

	return;
}


void Paramaterize::UpdatePara()
{
	for (int i = 0; i < inside_count_; i++)
	{
		V* v = inside_points_[i];
		v->pos = vecf3({ X(i, 0), X(i, 1), X(i, 2) });
	}
}
