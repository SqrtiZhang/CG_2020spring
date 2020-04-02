#pragma once

#include <Basic/HeapObj.h>
//#include <Engine/Primitive/MassSpring.h>
#include <Eigen/Core>
#include <Eigen/Sparse>
#include <Eigen/SparseLU>
#include <UGM/UGM>
#include <Eigen/Cholesky>
#include <map>
#include <set>

namespace Ubpa {
	class Simulate : public HeapObj {
	public:
		Simulate(const std::vector<pointf3>& plist,
			const std::vector<unsigned>& elist) {
			edgelist = elist;
			this->positions.resize(plist.size());
			this->xk.resize(plist.size());
			this->xk_1.resize(plist.size());

			for (int i = 0; i < plist.size(); i++)
			{
				for (int j = 0; j < 3; j++)
				{
					this->positions[i][j] = plist[i][j];
				}
			}
		};
	public:
		static const Ptr<Simulate> New(const std::vector<pointf3>& plist,
			const std::vector<unsigned>& elist) {
			return Ubpa::New<Simulate>(plist, elist);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init();
		//bool Init();

		// call it after Init()
		bool Run();

		const std::vector<pointf3>& GetPositions() const { return positions; };

		const double GetStiff() { return stiff; };
		void SetStiff(double k) { stiff = k; Init(); };
		const double GetTimeStep() { return h; };
		void SetTimeStep(double k) { h = k; Init(); };
		std::vector<unsigned>& GetFix() { return this->fixed_id; };
		void SetFix(const std::vector<unsigned>& f) { this->fixed_id = f; Init(); };
		const std::vector<pointf3>& GetVelocity() { return velocity; };
		//void SetVelocity(const std::vector<pointf3>& v) { velocity = v; };

		void SetLeftFix();


	private:
		// kernel part of the algorithm
		void SimulateOnce();

	private:
		double h = 0.03f;
		double stiff;
		std::vector<unsigned> fixed_id;  //fixed point id


		//mesh data
		std::vector<unsigned> edgelist;


		//simulation data
		std::vector<pointf3> positions;
		std::vector<pointf3> velocity;

	private:
		double g;
		std::vector<double> force_ext;
		std::vector<double> force_int;
		std::vector<double> mass;
		std::vector<double> l;

		std::vector<double> xk;
		std::vector<double> x;
		std::vector<double> xk_1;
		std::vector<double> b;
		std::vector<double> gx;

		Eigen::MatrixXd gx_m;
		Eigen::MatrixXd K;
		Eigen::MatrixXd inverG;
		Eigen::SparseMatrix<double> diff;
		//Eigen::MatrixXd diff_;
		//Eigen::SparseLU<Eigen::SparseMatrix<double>> LU_;

		std::set<int> fix;


	public:
		void SetInitG();
		void CacForce();
		void GetSet();
		void CacX();
		void CacGX();
		void CacK();
		void CacGxM();
		void CacDiff();
		bool isconv();
		void CacV();
		void UpdateX();
		void UpdatePos();
		void SetFix();


	private:
		int iter;
		std::vector<double> d;
		std::vector<double> y_;
		std::vector<double> xx;
		Eigen::MatrixXd J;
		Eigen::MatrixXd L;
		Eigen::MatrixXd M;

		Eigen::SparseMatrix<double> A;
		Eigen::SimplicialLLT<Eigen::SparseMatrix<double> > LLT_;

		bool isfast;

	public:
		void CacJ();
		void CacL();
		void CacA();
		void Global_CacX();
		void Local_CacD();

		void  SetFast();
	};
}
