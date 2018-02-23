
# include "my_viewer.h"

# include <sigogl/ui_button.h>
# include <sigogl/ui_radio_button.h>
# include <sig/sn_primitive.h>
# include <sig/sn_transform.h>
# include <sig/sn_manipulator.h>

# include <sigogl/ws_run.h>
GsVec light_vec(0, 20.0f, 20.0f);
bool pause = false;

MyViewer::MyViewer ( int x, int y, int w, int h, const char* l ) : WsViewer(x,y,w,h,l)
{
	_nbut=0;
	_animating=false;
	build_ui ();
	build_scene ();
}

void MyViewer::build_ui ()
{
	UiPanel *p, *sp;
	UiManager* uim = WsWindow::uim();
	p = uim->add_panel ( "", UiPanel::HorizLeft );
	p->add ( new UiButton ( "View", sp=new UiPanel() ) );
	{	UiPanel* p=sp;
		p->add ( _nbut=new UiCheckButton ( "Normals", EvNormals ) ); 
	}
	p->add ( new UiButton ( "Animate", EvAnimate ) );
	p->add ( new UiButton ( "Exit", EvExit ) ); p->top()->separate();
}

void MyViewer::add_model ( SnShape* s, GsVec p )
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation ( p );
	manip->initial_mat ( m );

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);

	rootg()->add(manip);
}

void MyViewer::add_polygon(SnNode* n, GsPnt p)
{
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation(p);
	manip->initial_mat(m);
	manip->child(n);
	rootg()->add(manip);
}

void MyViewer::add_shadow(SnShape* s, GsVec p)
{
	GsMat shadow;
	GsMat shadowMod;

	shadow.setl1(1, -light_vec.x / light_vec.y, 0, 2);
	shadow.setl2(0, 0, 0, -2);
	shadow.setl3(0, -light_vec.z / light_vec.y, 1, -2);
	shadow.setl4(0, 0, 0, 1);
	//shadowMod.mult(shadow, m);
	
	SnManipulator* manip = new SnManipulator;
	GsMat m;
	m.translation(p);
	m.mult(shadow, m);
	manip->initial_mat(m);

	SnGroup* g = new SnGroup;
	SnLines* l = new SnLines;
	l->color(GsColor::orange);
	g->add(s);
	g->add(l);
	manip->child(g);

	rootg()->add(manip);
}

void MyViewer::build_scene ()
{
	SnPrimitive* p;
	//GsPolygon* g;

	//Tall arrow cylinder (0) -> (4)
	p = new SnPrimitive(GsPrimitive::Cylinder,0.05f,0.05f,0.6f);
	p->prim().material.diffuse=GsColor::blue;
	add_model ( p, GsVec(0,0.6f,0) );

	//short arrow cylinder (1) -> (6)
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.05f, 0.05f, 0.3f);
	p->prim().material.diffuse = GsColor::darkblue;
	add_model(p, GsVec(0, 0.3f, 0));

	//short arrowhead (2) -> (7)
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.15f, 0.01f, 0.15f);
	p->prim().material.diffuse = GsColor::darkblue;
	add_model(p, GsVec(0, 0.65f, 0));
	
	//tall arrowhead (3) -> (5)
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.15f, 0.01f, 0.15f);
	p->prim().material.diffuse = GsColor::blue;
	add_model(p, GsVec(0, 1.35f, 0));

	//add shadow models
	
	//Tall arrow Cylinder (4)
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.05f, 0.05f, 0.6f);
	p->prim().material.diffuse = GsColor::black;
	add_shadow(p, GsVec(0, 0.6f, 0));
	//Tall arrowhead (5)
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.15f, 0.01f, 0.15f);
	p->prim().material.diffuse = GsColor::black;
	add_shadow(p, GsVec(0, 1.35f, 0));
	//Short arrow cylinder (6)
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.05f, 0.05f, 0.3f);
	p->prim().material.diffuse = GsColor::black;
	add_shadow(p, GsVec(0, 0.3f, 0));
	//short arrowhead (7)
	p = new SnPrimitive(GsPrimitive::Cylinder, 0.15f, 0.01f, 0.15f);
	p->prim().material.diffuse = GsColor::black;
	add_shadow(p, GsVec(0, 0.65f, 0));

	//Clock Circle
	SnGroup* g = new SnGroup;

	SnPolygons* snp = new SnPolygons;
	snp->color(GsColor::random(), GsColor::random(), GsColor::random());
	GsPolygons* r = snp->polygons();
	r->push();
	r->top().circle_approximation(GsPnt2(0, 0), 2.0f, 50);
	r->top().open(true);
	g->add(snp);

	add_polygon(g, GsVec2(0, 0));

	
}

// Below is an example of how to control the main loop of an animation:
void MyViewer::run_animation ()
{
	if (_animating) return; // avoid recursive calls
	_animating = true;

	double frdt = 1.0/30.0; // delta time to reach given number of frames per second
	double v = 4; // target velocity is 1 unit per second
	double t=0, lt=0, t0=gs_time();
	float angle = 0.0023f / 4 * 7; //Angle change of the tall arrow
	float dangle = 0.0023f / 4 * 7;
	float dangle2 = 0.0023f / 60 / 4 * 7; //Angle change of the short arrow
	float angle2 = 0.0023f / 60 / 4 * 7;


	do // run for a while:
	{	
		GsMat shadow, p;

		shadow.setl1(1, -light_vec.x / light_vec.y, 0, 2);
		shadow.setl2(0, 0, 0, -2);
		shadow.setl3(0, -light_vec.z / light_vec.y, 1, -2);
		shadow.setl4(0, 0, 0, 1);
		//p.translation(2, -2, -2);
		//shadow.mult(p, shadow);
	
		while ( t-lt<frdt ) { ws_check(); t=gs_time()-t0; } // wait until it is time for next frame
		
		//Short arrow cylinder
		SnManipulator* manip = rootg()->get<SnManipulator>(1); // access one of the manipulators
		GsMat m,m1;
		GsMat a, b, c;
	
		a.translation(0, 0, 0);	
		b.rotz(angle2);	
		c.translation(0, 0.3f, 0);

		a.mult(a, b);
		a.mult(a, c);
		m.set(a);
		angle2 -= dangle2;
		
		manip->initial_mat(m);
		
		//Short arrow cylinder shadow
		manip = rootg()->get<SnManipulator>(6);
		//m1.translation(0, -4.7f, 0);
		//m.mult(m, m1);
		m.mult(shadow, m);
		manip->initial_mat(m);



		//Short arrowhead
		manip = rootg()->get<SnManipulator>(2); // access one of the manipulators
		GsMat a1, b1, c1;

		a1.translation(0, 0, 0);
		b1.rotz(angle2);
		c1.translation(0, 0.65f, 0);

		a1.mult(a1, b1);
		a1.mult(a1, c1);
		m.set(a1);

		manip->initial_mat(m);
		
		//Short arrowhead shadow
		manip = rootg()->get<SnManipulator>(7);
		m.mult(shadow, m);
		manip->initial_mat(m);
		angle2 -= dangle2;


		//Tall arrowhead
		manip = rootg()->get<SnManipulator>(3); // access one of the manipulators
		GsMat a2, b2, c2;

		a2.translation(0, 0, 0);
		b2.rotz(angle);
		c2.translation(0, 1.35f, 0);

		a2.mult(a2, b2);
		a2.mult(a2, c2);
		m.set(a2);

		manip->initial_mat(m);
		
		//Tall arrowhead shadow
		manip = rootg()->get<SnManipulator>(5);
		m.mult(shadow, m);
		manip->initial_mat(m);
		angle -= dangle;


		//Tall arrow Cylinder
		manip = rootg()->get<SnManipulator>(0); // access one of the manipulators
		GsMat a3, b3, c3;

		a3.translation(0, 0, 0);
		b3.rotz(angle);
		c3.translation(0, 0.6f, 0);

		a3.mult(a3, b3);
		a3.mult(a3, c3);
		m.set(a3);

		manip->initial_mat(m);

		//Tall arrow Cylinder Shadow
		manip = rootg()->get<SnManipulator>(4);
		m.mult(shadow, m);
		manip->initial_mat(m);
		
		angle -= dangle;
		
		render(); // notify it needs redraw
		ws_check(); // redraw now
		
	
	}	while (!pause );
	
	_animating = false;
}

void MyViewer::show_normals ( bool b )
{
	// Note that primitives are only converted to meshes in GsModel
	// at the first draw call.
	GsArray<GsVec> fn;
	SnGroup* r = (SnGroup*)root();
	for ( int k=0; k<r->size(); k++ )
	{	SnManipulator* manip = r->get<SnManipulator>(k);
		SnShape* s = manip->child<SnGroup>()->get<SnShape>(0);
		SnLines* l = manip->child<SnGroup>()->get<SnLines>(1);
		if ( !b ) { l->visible(false); continue; }
		l->visible ( true );
		if ( !l->empty() ) continue; // build only once
		l->init();
		if ( s->instance_name()==SnPrimitive::class_name )
		{	GsModel& m = *((SnModel*)s)->model();
			m.get_normals_per_face ( fn );
			const GsVec* n = fn.pt();
			float f = 0.33f;
			for ( int i=0; i<m.F.size(); i++ )
			{	const GsVec& a=m.V[m.F[i].a]; l->push ( a, a+(*n++)*f );
				const GsVec& b=m.V[m.F[i].b]; l->push ( b, b+(*n++)*f );
				const GsVec& c=m.V[m.F[i].c]; l->push ( c, c+(*n++)*f );
			}
		}  
	}
}

int MyViewer::handle_keyboard ( const GsEvent &e )
{
	int ret = WsViewer::handle_keyboard ( e ); // 1st let system check events
	if ( ret ) return ret;

	switch ( e.key )
	{	case GsEvent::KeyEsc : gs_exit(); return 1;
		case 'n' : { bool b=!_nbut->value(); _nbut->value(b); show_normals(b); return 1; }
		case GsEvent::KeyEnter:run_animation(); return 1;
		case GsEvent::KeyCodes(32): {
			if (pause == true) {
				pause = false;
				run_animation();
			}
			else {
				pause = true;

			}

			return 1;
		}
		case GsEvent::KeyCodes(119): {
			light_vec.y += 5.0f;
			return 1;
		}
		case GsEvent::KeyCodes(115): {
			light_vec.y -= 5.0f;
			return 1;
		}
		case GsEvent::KeyCodes(113): {
			light_vec.x += 5.0f;
			return 1;
		}
		case GsEvent::KeyCodes(97): {
			light_vec.x -= 5.0f;
			return 1;
		}
		case GsEvent::KeyCodes(101): {
			light_vec.z += 5.0f;
			return 1;
		}
		case GsEvent::KeyCodes(100): {
			light_vec.z -= 5.0f;
			return 1;
		}
		default: gsout<<"Key pressed: "<<e.key<<gsnl;
	}

	return 0;
}

int MyViewer::uievent ( int e )
{
	switch ( e )
	{	case EvNormals: show_normals(_nbut->value()); return 1;
		case EvAnimate: run_animation(); return 1;
		case EvExit: gs_exit();
	}
	return WsViewer::uievent(e);
}
