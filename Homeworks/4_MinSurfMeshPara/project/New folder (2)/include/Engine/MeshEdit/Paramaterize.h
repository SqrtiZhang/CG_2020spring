#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <Eigen/Core>
#include <Eigen/SparseLU>

#include <unordered_map>
#include <vector>
#include <math.h>

namespace Ubpa {
	class TriMesh;
	//class MinSurf;

	// mesh boundary == 1
	class Paramaterize : public HeapObj {
	public:
		  Paramaterize(Ptr<TriMesh> triMesh);
	public:
		static const Ptr<Paramaterize> New(Ptr<TriMesh> triMesh) {
			return Ubpa::New<Paramaterize>(triMesh);
		}


	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh);

		bool Run();
		void SetTex(int m);
		void SetWMethod(int m);
		
		
	private:
		// kernel part of the algorithm
		void Para();

	public:
		class V;
		class E;
		class P;
		class V : public TVertex<V, E, P> {
		public:
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> { };
		class P :public TPolygon<V, E, P> { };

	public:
		Ptr<HEMesh<V>> ReturnPos();

	private:
		//friend class Paramaterize;

		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		bool tex_;
		int type_;

	private:
		Eigen::SparseMatrix<double> L_;
		Eigen::MatrixX3d delta_;
		Eigen::SparseLU<Eigen::SparseMatrix<double>> LU_;
		Eigen::MatrixX3d X;
		std::vector<V*> inside_points_;
		std::vector<V*> boundary_points_;
		std::unordered_map<int, int> map_between_inside_;
		int inside_count_;
		int boundary_count_;

	protected:
		void ParaU();
		void ParaTan();
		void FindInside();
		void SetBoundary();
		void GetMap();
		//void BuildLMat();
		//void BuildDeltaMat();
		void UpdatePara();

		
	};
}
