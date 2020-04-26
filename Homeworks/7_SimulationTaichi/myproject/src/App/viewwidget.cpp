#include "viewwidget.h"
#include "taichi.h"

ViewWidget::ViewWidget(QWidget* parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	draw_status_ = false;
	shape_ = NULL;
	setSource = false;
	type_ = Shape::kDefault;
    current_color=Qt::black;
    current_width=1;
	current_vel.clear();
	current_vel.push_back(0.0);
	current_vel.push_back(0.0);
	stop_simulate = false;
	pause_simulate = false;
    stop_polygon=true;
    isundo=false;

	young_view = 1e4f;
}

ViewWidget::~ViewWidget()
{
}

void ViewWidget::setLine()
{
    type_ = Shape::kLine;
}

void ViewWidget::setRect()
{
	type_ = Shape::kRect;
}

void ViewWidget::setEllipse()
{
    type_ = Shape::kEllipse;
}

void ViewWidget::setPoly()
{
    type_=Shape::kPoly;
}

void ViewWidget::setFreeHand()
{
    type_=Shape::kFreeHand;
}

void ViewWidget::mousePressEvent(QMouseEvent* event)
{
    isundo=false;

    if(Qt::RightButton==event->button())
    {
        stop_polygon=true;
        return;
    }

	if (Qt::LeftButton == event->button())
    {
        if(type_ ==Shape:: kPoly)
        {
            if(stop_polygon==true||shape_list_.empty())
            {
                shape_ = new Poly;
                Poly *tmp=dynamic_cast<Poly *>(shape_);
                QPoint *p = new QPoint(event->pos());
                tmp->Push(p);
                QPoint *p2 = new QPoint(event->pos() + QPoint(0, 1));
                tmp->Push(p2);
                shape_list_.push_back(shape_);
                shape_->set_line(current_color,current_width);
                stop_polygon = false;
            }
            else
            {
                if(type_ !=Shape:: kPoly)
                {
                    stop_polygon=true;
                    return;
                }
                QPoint *p = new QPoint(event->pos());
                dynamic_cast<Poly *>(shape_)->Push(p);
            }
            if (shape_ != NULL)
            {
                draw_status_ = true;
                start_point_ = end_point_ = event->pos();
                shape_->set_start(start_point_);
                shape_->set_end(end_point_);
            }
            update();
            return;
         }

        stop_polygon=true;

        switch (type_)
        {
        case Shape::kLine:
            shape_ = new Line();
            break;
        case Shape::kDefault:
            break;
        case Shape::kRect:
            shape_ = new Rect();
            break;
        case Shape::kEllipse:
            shape_ = new Elli();
            break;
        case Shape::kFreeHand:{
            shape_=new FreeHand();
            Shape *zero=new FreeHand;
            zero->set_start(QPoint(0,0));
            zero->set_end(QPoint(0,0));
            shape_list_.push_back(zero);
            break;}
        default:
            break;
        }

        if (shape_ != NULL)
        {
            draw_status_ = true;
            start_point_ = end_point_ = event->pos();
            shape_->set_start(start_point_);
			
            shape_->set_end(end_point_);
            shape_->set_line(current_color,current_width);
			shape_->set_material(current_Material);
			shape_->set_v(current_vel[0],current_vel[1]);
        }
        update();
   }

}

void ViewWidget::mouseMoveEvent(QMouseEvent* event)
{

	if (draw_status_ && shape_ != NULL)
	{
        if(type_ == Shape::kPoly)
        {
            Poly *tmp = dynamic_cast<Poly *>(shape_list_.back());
            QPoint *p = tmp->top();
            p->setX(event->x());
            p->setY(event->y());
            update();
            return;
        }
        end_point_ = event->pos();
        shape_->set_end(end_point_);
        if(type_==Shape::kFreeHand)
        {
            shape_=new Line();
            shape_->set_line(current_color,current_width);
            shape_list_.push_back(shape_);
            shape_->set_start(event->pos());
            shape_->set_end(event->pos());
        }
        update();
	}
}

void ViewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    if(type_==Shape::kFreeHand)
    {
        Shape *zero=new FreeHand;
        zero->set_start(QPoint(0,0));
        zero->set_end(QPoint(0,0));
        shape_list_.push_back(zero);
        return;
    }
    if (shape_ != NULL)
    {
        draw_status_ = false;
        if(type_!=Shape::kPoly)
        {
            shape_list_.push_back(shape_);
			if (setSource)
			{
				shape_source_.push_back(shape_);
				setSource = false;
			}
            shape_ = NULL;
        }

    }
}

void ViewWidget::paintEvent(QPaintEvent*)
{
    this->setAutoFillBackground(true);
    QPalette pal;
    if(!filename.isEmpty())
        pal.setBrush(QPalette::Window,QBrush(QPixmap(filename)));
    else
        pal.setColor(QPalette::Window,Qt::white);
    setPalette(pal);

	QPainter painter(this);

	for (int i = 0; i < shape_list_.size(); i++)
	{
		shape_list_[i]->Draw(painter);
	}

    if (shape_ != NULL && !isundo) {
		shape_->Draw(painter);
	}

	for (int i = 0; i < point_.size(); i++)
	{
		int color = point_color_[i];
		//std::cout << color <<"   "<< endl;
		//std::cout << (color >> 16 )<<"  " << endl;
		//std::cout << ((color >> 16) & 255) << " " << ((color >> 8) & 255 )<< " " << (color & 255)<<endl;
		//std::cout << QColor(color >> 16 & 255, color >> 8 & 255, color & 255).green();
		painter.setPen(QPen(QColor(color>>16 & 255 ,color >> 8 & 255,color & 255), 2));
		painter.drawPoint(point_[i]);
	}

	update();
}


void ViewWidget::setColor(QColor c)
{
    current_color=c;
}

void ViewWidget::setMaterial(int m)
{
	current_Material = m;
	//std::cout << m << endl;
}

void ViewWidget::setWidth(int w)
{
    current_width = w % 20;
}

void ViewWidget::setBackground()
{
    filename=QFileDialog::getOpenFileName(this,"打开文件",QDir::currentPath());
}

void ViewWidget::Undo()
{
	stop_polygon = true;
    if(!shape_list_.empty())
    {
        isundo=true;

        if(typeid(*shape_list_.back())==typeid(FreeHand))
        {
            redo_shape_list.push_back(shape_list_.back());
            shape_list_.pop_back();

            while(!shape_list_.empty())
            {
                if(shape_list_.back()->get_start()==QPoint(0,0) &&
                        shape_list_.back()->get_end()==QPoint(0,0))
                {
                    redo_shape_list.push_back(shape_list_.back());
                    shape_list_.pop_back();
                    break;
                }

                redo_shape_list.push_back(shape_list_.back());
                shape_list_.pop_back();
            }
        }
        else
        {
            redo_shape_list.push_back(shape_list_.back());
            shape_list_.pop_back();
        }
    }
    update();
}

void ViewWidget::Redo()
{
	stop_polygon = true;
    if(!redo_shape_list.empty())
    {
        if(typeid(*redo_shape_list.back())==typeid(FreeHand))
        {
            shape_list_.push_back(redo_shape_list.back());
            redo_shape_list.pop_back();

            while(!redo_shape_list.empty())
            {
                if(redo_shape_list.back()->get_start()==QPoint(0,0) &&
                        redo_shape_list.back()->get_end()==QPoint(0,0))
                {
                    shape_list_.push_back(redo_shape_list.back());
                    redo_shape_list.pop_back();
                    break;
                }
                shape_list_.push_back(redo_shape_list.back());
                redo_shape_list.pop_back();
            }
        }
        else
        {
            shape_list_.push_back(redo_shape_list.back());
            redo_shape_list.pop_back();
        }
    }
    update();
}

void ViewWidget::Clear()
{
    current_color=Qt::black;
    current_width=1;
    shape_list_.clear();
    redo_shape_list.clear();
    stop_polygon=true;
    filename="";
    update();
}

void ViewWidget::Sim_Stop()
{ 
	stop_simulate = true; 
	shape_source_.clear();
	point_.clear();
	shape_list_.clear();
	point_color_.clear();
};

void ViewWidget::Sim_Pause()
{
	pause_simulate = true;
};

void ViewWidget::setYoung(QString s)
{
	//pause_simulate = true;
	young_view = s.toFloat();
	//std::cout << young_view << endl;
};

void ViewWidget::setVx(QString s)
{
	current_vel[0] = -s.toFloat();

};

void ViewWidget::setVy(QString s)
{
	current_vel[1] = s.toFloat();

};

void ViewWidget::Sim_Source()
{
	setSource = true;
}

#define TC_IMAGE_IO
using namespace taichi;
using Vec = Vector2;
using Mat = Matrix2;

const int n = 80;

const real dt = 1e-4_f;
const real frame_dt = 1e-3_f;
const real dx = 1.0_f / n;
const real inv_dx = 1.0_f / dx;

// Snow material properties
const auto particle_mass = 1.0_f;
const auto vol = 1.0_f;        // Particle Volume
const auto hardening = 10.0_f; // Snow hardening factor
auto E = 1e4_f;          // Young's Modulus
const auto nu = 0.2_f;         // Poisson ratio
const bool plastic = true;

// Initial Lamé parameters
const real mu_0 = E / (2 * (1 + nu));
const real lambda_0 = E * nu / ((1 + nu) * (1 - 2 * nu));

struct Particle {
	// Position and velocity
	Vec x, v;
	// Deformation gradient
	Mat F;
	// Affine momentum from APIC
	Mat C;
	// Determinant of the deformation gradient (i.e. volume)
	real Jp;
	// Color
	int c;
	int ptype;/*0: fluid 1: jelly 2: snow 3:wood brick*/

	Particle(Vec x, int c, int ptype, Vec v = Vec(0)) :
		x(x),
		v(v),
		F(1),
		C(0),
		Jp(1),
		c(c),
		ptype(ptype){}
};

std::vector<Particle> particles;

Vector3 grid[n + 1][n + 1];

void advance(real dt) {
	// Reset grid
	std::memset(grid, 0, sizeof(grid));

	// P2G
	for (auto& p : particles) {
		// element-wise floor
		Vector2i base_coord = (p.x * inv_dx - Vec(0.5f)).cast<int>();

		Vec fx = p.x * inv_dx - base_coord.cast<real>();

		// Quadratic kernels [http://mpm.graphics Eqn. 123, with x=fx, fx-1,fx-2]
		Vec w[3] = {
		  Vec(0.5) * sqr(Vec(1.5) - fx),
		  Vec(0.75) - sqr(fx - Vec(1.0)),
		  Vec(0.5) * sqr(fx - Vec(0.5))
		};

		// Compute current Lamé parameters [http://mpm.graphics Eqn. 86]
		auto e = std::exp(hardening * (1.0f - p.Jp));
		if(p.ptype == 1) 
			e = 0.3;
		if (p.ptype == 3)
			e = 1;
		auto mu = mu_0 * e, lambda = lambda_0 * e;
		if (p.ptype == 0) 
			mu = 0;
		

		// Current volume
		real J = determinant(p.F);

		// Polar decomposition for fixed corotated model
		Mat r, s;
		polar_decomp(p.F, r, s);

		// [http://mpm.graphics Paragraph after Eqn. 176]
		real Dinv = 4 * inv_dx * inv_dx;
		// [http://mpm.graphics Eqn. 52]
		auto PF = (2 * mu * (p.F - r) * transposed(p.F) + lambda * (J - 1) * J);

		// Cauchy stress times dt and inv_dx
		auto stress = -(dt * vol) * (Dinv * PF);

		// Fused APIC momentum + MLS-MPM stress contribution
		// See http://taichi.graphics/wp-content/uploads/2019/03/mls-mpm-cpic.pdf
		// Eqn 29
		auto affine = stress + particle_mass * p.C;

		// P2G
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				auto dpos = (Vec(i, j) - fx) * dx;
				// Translational momentum
				Vector3 mass_x_velocity(p.v * particle_mass, particle_mass);
				grid[base_coord.x + i][base_coord.y + j] += (
					w[i].x * w[j].y * (mass_x_velocity + Vector3(affine * dpos, 0))
					);
			}
		}
	}

	// For all grid nodes
	for (int i = 0; i <= n; i++) {
		for (int j = 0; j <= n; j++) {
			auto& g = grid[i][j];
			// No need for epsilon here
			if (g[2] > 0) {
				// Normalize by mass
				g /= g[2];
				// Gravity
				g += dt * Vector3(0, -200, 0);

				// boundary thickness
				real boundary = 0.05;
				// Node coordinates
				real x = (real)i / n;
				real y = real(j) / n;
				//cout << x << endl;
				// Sticky boundary
				if (x < boundary || x > 1 - boundary || y > 1 - boundary) {
					g = Vector3(0);
				}
				// Separate boundary
				if (y < boundary) {
					g[1] = std::max(0.0f, g[1]);
				}
			}
		}
	}

	// G2P
	for (auto& p : particles) {
		// element-wise floor
		Vector2i base_coord = (p.x * inv_dx - Vec(0.5f)).cast<int>();
		Vec fx = p.x * inv_dx - base_coord.cast<real>();
		Vec w[3] = {
					Vec(0.5) * sqr(Vec(1.5) - fx),
					Vec(0.75) - sqr(fx - Vec(1.0)),
					Vec(0.5) * sqr(fx - Vec(0.5))
		};

		p.C = Mat(0);
		p.v = Vec(0);

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				auto dpos = (Vec(i, j) - fx);
				auto grid_v = Vec(grid[base_coord.x + i][base_coord.y + j]);
				auto weight = w[i].x * w[j].y;
				// Velocity
				p.v += weight * grid_v;
				// APIC C
				p.C += 4 * inv_dx * Mat::outer_product(weight * grid_v, dpos);
			}
		}

		// Advection
		p.x += dt * p.v;

		// MLS-MPM F-update
		auto F = (Mat(1) + dt * p.C) * p.F;

		if (p.ptype == 0) 
		{ p.F = Mat(1) * sqrt(determinant(F));}
		else if (p.ptype == 1 || p.ptype==3) 
		{ p.F = F; }
		else if (p.ptype == 2) 
		{
			Mat svd_u, sig, svd_v;
			svd(F, svd_u, sig, svd_v);
			for (int i = 0; i < 2 * int(plastic); i++)                // Snow Plasticity
				sig[i][i] = clamp(sig[i][i], 1.0_f - 2.5e-2_f, 1.0_f + 7.5e-3_f);
			real oldJ = determinant(F);
			F = svd_u * sig * transposed(svd_v);
			real Jp_new = clamp(p.Jp * oldJ / determinant(F), 0.6_f, 20.0_f);
			p.Jp = Jp_new; 
			p.F = F;
		}
	}
}

// Seed particles with position and color
void add_object(Vec start, Vec center,long long c,int ptype,int shape, Vec end, Vec velocity = Vec(0.0_f),int num=1000)
{
	if (shape == 2)//rect
	{
		float w = abs((end - start).x);
		float h = abs((end - start).y);
		for (int i = 0; i < num; i++) 
		{
			Vec r1 = Vec::rand() * 2.0f - Vec(1);
			Vec r2 = Vec::rand() * 2.0f - Vec(1);
			particles.push_back(Particle(Vec(r1.x / 2.0 * w, r2.y / 2.0 * h) + center, c, ptype,velocity));
		}
	}
	else//ball
	{
		for (int i = 0; i < num; )
		{
			float a = abs((start - end).x) / 2.0;
			float b = abs((start - end).y )/ 2.0;
			float radius =max( abs((start - end).x), abs((start - end).y));
			auto pos = (Vec::rand() * 2.0_f - Vec(1)) * radius;
			if (pos.x * pos.x/(a*a) + pos.y * pos.y/(b*b) < 1)
			{
				particles.push_back(Particle(pos + center, c, ptype, velocity));
				i++;
			}
		}
	}
	
}

void ViewWidget::Sim_Start()
{
	E = young_view;
	if (stop_simulate == true)
		particles.clear();
	stop_simulate = false;
	pause_simulate = false;
	for (int i = 0; i < shape_list_.size(); i++)
	{
		int type = shape_list_[i]->shape_type();
		QPoint t = shape_list_[i]->get_start() + shape_list_[i]->get_end();
		Vec start = Vec(1 - shape_list_[i]->get_start().x() / 1200_f, 1 - shape_list_[i]->get_start().y() / 800_f);
		Vec end = Vec(1 - shape_list_[i]->get_end().x() / 1200_f, 1 - shape_list_[i]->get_end().y() / 800_f);
		Vec vel = Vec(shape_list_[i]->get_v().x(), shape_list_[i]->get_v().y());
		long long c = ((shape_list_[i]->get_color().red()<<16)+ (shape_list_[i]->get_color().green()<<8)+ shape_list_[i]->get_color().blue());
		//cout << (shape_list_[i]->get_color().red() << 16) << endl;
		//cout << (shape_list_[i]->get_color().green()<< 8 )<< endl;
		//cout << (shape_list_[i]->get_color().blue()) << endl;
		add_object(start, Vec(1 - t.x() / 2_f / 1200_f, 1 - t.y() / 2_f / 800_f), c, shape_list_[i]->material_type(),type, end,vel);
		//cout << c << endl;
		//add_object(start, Vec(1 - t.x() / 2_f / 800_f, 1 - t.y() / 2_f / 800_f), 0x00a8ff, 3, 0, end);
		
	}

	int frame = 0;
	std::vector<int>a;
	// Main Loop
	for (int step = 0;( !stop_simulate) &&( !pause_simulate); step++) 
	{
		// Advance simulation
		advance(dt);
		//point_.clear();
		// Visualize frame
		if (step % int(frame_dt / dt) == 0) {
			point_.clear();
			point_color_.clear();
			for (auto p : particles)
			{
				QPointF point((1-p.x[0])*1200,(1-p.x[1])*800);
				
				point_.push_back(point);
				point_color_.push_back(p.c);
				
			}
			repaint();
			for (int i = 0; i < shape_source_.size(); i++)
			{
				int type = shape_source_[i]->shape_type();
				auto t = shape_source_[i]->get_start() + shape_source_[i]->get_end();
				Vec start = Vec(1 - shape_source_[i]->get_start().x() / 1200_f, 1 - shape_source_[i]->get_start().y() / 800_f);
				Vec end = Vec(1 - shape_source_[i]->get_end().x() / 1200_f, 1 - shape_source_[i]->get_end().y() / 800_f);
				Vec vel = Vec(shape_source_[i]->get_v().x(), shape_source_[i]->get_v().y());
				int c = ((shape_source_[i]->get_color().red() << 16) + (shape_source_[i]->get_color().green() << 8) + shape_source_[i]->get_color().blue());
				add_object(start, Vec(1 - t.x() / 2_f / 1200_f, 1 - t.y() / 2_f / 800_f), c, shape_source_[i]->material_type(), type, end, vel,300);
				//QCoreApplication::processEvents();
					
			}
			repaint();
			shape_list_.clear();
			QApplication::processEvents();
		}
	}
	//shape_source_.clear();
	return;
}