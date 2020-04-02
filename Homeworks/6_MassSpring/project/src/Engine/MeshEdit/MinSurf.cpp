#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>

using namespace Ubpa;

using namespace std;
using namespace Eigen;

MinSurf::MinSurf(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

void MinSurf::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool MinSurf::Init(Ptr<TriMesh> triMesh) {
	Clear();

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

bool MinSurf::Run() {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	Minimize();

	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf3> positions;
	vector<unsigned> indice;
	positions.reserve(nV);
	indice.reserve(3 * nF);
	for (auto v : heMesh->Vertices())
		positions.push_back(v->pos.cast_to<pointf3>());
	for (auto f : heMesh->Polygons()) { // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}

	triMesh->Init(indice, positions);

	return true;
}

void MinSurf::Minimize() {
	// TODO
	//cout << "WARNING::MinSurf::Minimize:" << endl
	//     << "\t" << "not implemented" << endl;
	FindInside();
	GetMap();
	BuildLMat();
	BuildDeltaMat();

	LU_.compute(L_);
	X = LU_.solve(delta_);

	UpdateMin();
}


void MinSurf::FindInside()
{
	for (auto v : heMesh->Vertices())
		if (!v->IsBoundary())
			inside_points_.push_back(v);
	inside_count_ = inside_points_.size();
	return;
}

void MinSurf::GetMap()
{
	for (int i = 0; i < inside_count_; i++) 
		map_between_inside_.insert({ heMesh->Index(inside_points_[i]), i });
		//cout << heMesh->Index(inside_points_[i]) << endl;
	
	return;
}

void MinSurf::BuildLMat()
{
	L_.resize(inside_count_, inside_count_);

	vector<Eigen::Triplet<double> > coefficients;

	for (int i = 0; i < inside_count_; i++)
	{
		V* v=inside_points_[i];
		coefficients.push_back(Eigen::Triplet<double>(i, i, v->AdjVertices().size()));

		for (auto adj : v->AdjVertices())
		{
			int j = heMesh->Index(adj);
			if (!adj->IsBoundary())
			{
				j = map_between_inside_[j];
				coefficients.push_back(Eigen::Triplet<double>(i, j, -1));
			}
		}
	}
	L_.setFromTriplets(coefficients.begin(), coefficients.end());
}

void MinSurf::BuildDeltaMat()
{
	delta_ = Eigen::MatrixX3d::Zero(inside_count_, 3);

	for (int i = 0; i < inside_count_; i++)
	{
		V* v = inside_points_[i];
		
		for (auto adj : v->AdjVertices())
		{
			int j = heMesh->Index(adj);
			if (adj->IsBoundary())
			{
				delta_(i, 0) += adj->pos[0];
				delta_(i, 1) += adj->pos[1];
				delta_(i, 2) += adj->pos[2];
			}
		}
	}
}

void MinSurf::UpdateMin()
{
	for (int i = 0; i < inside_count_; i++)
	{
		V* v = inside_points_[i];
		v->pos = vecf3({ X(i, 0), X(i, 1), X(i, 2) });
	}
}