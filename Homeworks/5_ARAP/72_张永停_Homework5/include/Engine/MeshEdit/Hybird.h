#pragma once
#pragma once
#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <Eigen/Core>
#include <Eigen/SVD>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>

#include <unordered_map>
#include <vector>
#include <math.h>

#include <Engine/MeshEdit/Paramaterize.h>


namespace Ubpa {
	class TriMesh;
	//class MinSurf;

	// mesh boundary == 1
	class Hybird : virtual public Paramaterize {
	public:
		Hybird(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<Hybird> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<Hybird>(triMesh);
		}

	private:
		void ParaByHybird();

	private:
		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		bool tex_;
		//int type_;

	private:
		std::vector<Eigen::Vector2d> Xt_;
		std::vector<Eigen::Vector2d> Ut_;
		std::vector<Eigen::Matrix2d> Lt_;
		std::vector<double> Cot_;

		Eigen::SparseMatrix<double> Cof_;
		Eigen::MatrixX2d b_;
		Eigen::MatrixX2d X_;
		Eigen::SparseLU<Eigen::SparseMatrix<double>> LU_;

		std::map<THalfEdge<V, E, P>*, int> he_to_index;

		int pos_not_move;
		Eigen::Vector2d pos_coor;

		std::vector<double> root;

		double lambda;

	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh);
		bool Run();
		void SetTex(int m);
		//void SetWMethod(int m);

	protected:
		void initUt();
		void GetMap();
		void CacXtAndCot();
		//void CacUt();
		void CacLt();
		void CacB();
		void CacCof();
		void UpdateUt();

		void SetPos();

		void UpdateHeMesh();

		void Normlize();

		double CacCot(THalfEdge<V, E, P>* he);

		void ShengJin(double a, double b, double c, double d);
	};
}

