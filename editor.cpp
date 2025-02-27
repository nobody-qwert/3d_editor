#include "editor.h"




/********************************GLOBAL VARIABLES**********************************/

EDITOR	g_editor;


/********************************FUNCTIONS*****************************************/


/**********************************OBJECT CLASS************************************/


OBJECT::OBJECT() {
	numpoints = numtriangles = 0;

	vbuff3	= NULL;
	vbuff2	= NULL;
	tbuff	= NULL;
	p		= NULL;

	tex_size = CVector2(1.0f, 1.0f);

	NewTextureInfo(texture);
	initialized = false;
}

OBJECT::~OBJECT() {	
}


UINT OBJECT::GetGLTexID() {
	if (texture.num_textures > 0) {
		return texture.gl_tex_ID[0];
	}

	return g_editor.GetDefaultTexture();
}

void OBJECT::Drag(CVector3 v) {
	for (int i=0; i<NUM_CONTROLS[type]; i++) {
		p[i]=p[i]+v;
	}
	
	for (i=0; i<numpoints; i++) {
		vbuff3[i]=vbuff3[i]+v;
	}

	centrum=centrum+v;
}

void OBJECT::Reset(bool flag) {
	numpoints = numtriangles = 0;

	vbuff3 = NULL;
	vbuff2 = NULL;
	tbuff = NULL;
	p = NULL;

	initialized = false;
	if (flag) {
		tex_shift = CVector2(0,0);
		tex_size = CVector2(1,1);
		NewTextureInfo(texture);
	}
}


void OBJECT::SetTextureImage(TEXTURELIST tl) {
	AddTexture(texture, tl);
	tex_size = CVector2(TEXTURE_UNIT/tl.width, TEXTURE_UNIT/tl.height);
}

void OBJECT::SetType(int st) {
	if (st!=ST_BULB) type = ST_BOX;
	type = st;
}

void OBJECT::SetData(SHAPE shape) {
	if (p!=NULL) free(p);
	if (vbuff3!=NULL) free(vbuff3);
	if (vbuff2!=NULL) free(vbuff2);
	if (tbuff!=NULL) free(tbuff);

	initialized = true;

	centrum = shape.GetCenter();
	orientation = shape.GetOrientation();
	type = shape.GetType();
	numpoints = shape.GetNumPoints();
	numtriangles = shape.GetNumTriangles();
	smoothness = shape.GetSmoothness();
	//tex_shift = shape.tex_shift;
	//tex_size = shape.tex_size;

	p = new CVector3[NUM_CONTROLS[type]];
	vbuff3 = new CVector3[numpoints];
	vbuff2 = new CVector2[numpoints];
	tbuff = new Triangle[numtriangles];

	memcpy(p, shape.GetControlPointList(), NUM_CONTROLS[type]*sizeof(CVector3));	
	memcpy(vbuff3, shape.GetVertexList(), numpoints*sizeof(CVector3));
	memcpy(vbuff2, shape.GetTexCoordList(), numpoints*sizeof(CVector2));	
	memcpy(tbuff, shape.GetTriangleList(), numtriangles*sizeof(Triangle));
}

/**********************************EDITOR CLASS************************************/


void EDITOR::InitializeFonts() {
	HDC hDC = *((HDC *)glwindow.GetInfo(GI_HDC));

	glwindow.SetFont(&font);

	glwindow.MakeCurrent(PROJECTIONWINDOW);
	font.CreateOpenGLFont("Arial", GL_FONT_HEIGHT, hDC);

	glwindow.MakeCurrent(MODELLWINDOW);
	font.CreateOpenGLFont("Arial", GL_FONT_HEIGHT, hDC);

	glwindow.MakeCurrent(TEXTUREWINDOW);
	font.CreateOpenGLFont("Arial", GL_FONT_HEIGHT, hDC);

	glwindow.MakeCurrent(ONEDWINDOW);
	font.CreateOpenGLFont("Arial", GL_FONT_HEIGHT, hDC);	
}


void EDITOR::PrintLog(char* text, ...)
{
	va_list arg_list;
	char buff1[MAXSTR], *buff2;

	va_start(arg_list, text);
		vsprintf(buff1, text, arg_list);
	va_end(arg_list);

	
	sprintf(buff1, "%s\r\n ", buff1);

	HWND hWnd = *((HWND *) glwindow.GetInfo(GI_CHWND, LOGWINDOW));
	int length = GetWindowTextLength(hWnd);

	buff2 = new char[length + MAXSTR];

	GetWindowText(hWnd, buff2, length);
	buff2[length] = 0;

	strcat(buff2, buff1);

	SetWindowText(hWnd, buff2);
}


float EDITOR::GetMousePos1D(bool snap) {
	POINT		pos;	
	float		y;
		
	pos = *(POINT *)glwindow.GetInfo(GI_CMOUSEPOS, ONEDWINDOW);
	y = pos.y;
	
	TransformWCTo1DPolar(&y, 1);

	if (snaptogrid && snap) {
		y -= (int)y%(int)(gridSize * scalemap);
	}

	y =  y / scalemap;

	return y;
}


CVector2 EDITOR::GetMousePos2D(bool snap) {
	POINT		pos;	
	float		x,y;
		
	pos = *(POINT *)glwindow.GetInfo(GI_CMOUSEPOS, PROJECTIONWINDOW);
	x = pos.x;
	y = pos.y;

	TransformWCTo2DPolar(&x, &y, 1);

	if (snaptogrid && snap) {
		x -= (int)x%(int)(gridSize * scalemap);
		y -= (int)y%(int)(gridSize * scalemap);
	}
	
	x = x / scalemap;
	y = y / scalemap;

	return CVector2(x,y);
}


void EDITOR::InsertLight() {
	if (!tmpbrush.active) return;

	bulb_sel[numbulbsel] = numbulbs;
	numbulbsel++;

	bulb[numbulbs].pos	=	CVector3(tmpbrush.start[_X], tmpbrush.start[_Y], tmpbrush.start[_Z]);
	bulb[numbulbs].r	=	255;
	bulb[numbulbs].g	=	255;
	bulb[numbulbs].b	=	255;
	bulb[numbulbs].brightness	=	1.0f;
	bulb[numbulbs].cutoff		=	0.0f;
	bulb[numbulbs].range		=	256.0f;
	numbulbs++;

	tmpobject.FreeMemory();
	tmpbrush.active = false;

}


void EDITOR::DeleteObject(int idx, bool freedata) {
	if (idx<0 || idx>=numobjects) return;
	
	if (freedata) object[idx].FreeMemory();
	else object[idx].Reset();

	for (int i=idx; i<numobjects-1; i++)
		object[i] = object[i+1];

	// free up the last element
	object[numobjects-1].Reset();

	numobjects--;
}


void EDITOR::ScaleMap(float scale) {
	scalemap *= scale;
	if (scalemap < 1.0f / (1<<NUM_SCALES)) scalemap *= 2.0f;
	if (scalemap > (1<<NUM_SCALES)) scalemap /= 2.0f;
}

void EDITOR::ScaleTexture(float u, float v) {
	for (int i=0; i<numbrushes; i++) {
		object[brush[i].object].ScaleTexture(u,v);
	}

	if (tmpbrush.active) {
		tmpobject.ScaleTexture(u,v);
	}
}

void EDITOR::ShiftTexture(float u, float v) {
	for (int i=0; i<numbrushes; i++) {
		object[brush[i].object].ShiftTexture(u,v);
	}

	if (tmpbrush.active) {
		tmpobject.ShiftTexture(u,v);
	}
}


void EDITOR::SelectInside() {
	if (numbrushes>0 || !tmpbrush.active) return;

	numbrushes = 0;

	for (int i=0; i<numobjects; i++) {
		if (IsBoxInBox2D(object[i].GetBBoxN(false), tmpbrush)) {
			brush[numbrushes] = object[i].GetBBoxO();
			brush[numbrushes].object = i;
			numbrushes++;
		}		
	}

	for (i=0; i<numbulbs; i++) {
		if (IsPointInBox2D(bulb[i].pos, tmpbrush)) {
			bulb_sel[numbulbsel] = i;
			numbulbsel++;
		}
	}

	if (numbulbsel>0) {
		if (numbrushes==0) {
			tmpobject.FreeMemory();
			tmpbrush.active = false;
		}
	}

	if (numbrushes>0) {
		rotate = false;
		dragvertices = false;
		scaleobjects = false;
		Actualize();
	}

	if (numbrushes>1) {
		tmpobject.FreeMemory();
		tmpbrush.active = false;
	} else if (numbrushes==1) {
		tmpobject.FreeMemory();
		tmpobject	= object[brush[0].object];
		tmpbrush	= tmpobject.GetBBoxO();

		numbrushes	= 0;

		DeleteObject(brush[0].object);
	} 
}

void EDITOR::Check_vCtr_Change() {

	POINT point;
	
	point = *((POINT *)glwindow.GetInfo(GI_CRDRAG, PROJECTIONWINDOW));
	
	vCtr[GA(VF,0)] = point.x;
	vCtr[GA(VF,1)] = point.y;
	
	point = *((POINT *)glwindow.GetInfo(GI_CRDRAG, ONEDWINDOW));

	vCtr[VF] = point.y;
}


GLWindow * EDITOR::GetGLWindow() {
	return &glwindow;
}

void EDITOR::DrawSelection() {
	float sx,sy,
		  ex,ey;
	sx = sel_start[0];
	sy = sel_start[1];
	ex = sel_end[0];
	ey = sel_end[1];

	Transform2DToPolar(&sx,&sy,1);
	Transform2DToPolar(&ex,&ey,1);
	
	glColor4f(0,0,1,0.9f);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA,GL_SRC_ALPHA);

	glBegin(GL_QUADS);
		glVertex2f(sx, sy);
		glVertex2f(sx, ey);
		glVertex2f(ex, ey);
		glVertex2f(ex, sy);
	glEnd();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
}

void EDITOR::DrawSelectionMark(OBJECT obj, BRUSH br, bool is2D) {
	if (!br.active || !obj.IsInitialized()) return;
	
	if (is2D) {
		glEnable(GL_LINE_STIPPLE);
		glLineStipple(1, 0x0f0f);

		if (dragvertices) {
			CVector3	*vbuff;
			float		v[3];
			int			num;

			vbuff = obj.GetControlPointList();
			num = obj.GetNumControlPoints();

			for (int i=0; i<num; i++) {
				v[_X] = vbuff[i].x;
				v[_Y] = vbuff[i].y;
				v[_Z] = vbuff[i].z;

				Transform2DToPolar(&v[GA(VF,0)], &v[GA(VF,1)], 1);

				glColor3f(0,1,0);

				for (int jj=0; jj<numselected; jj++)
					if (selected[jj][0]==br.object && selected[jj][1]==i) {
						glColor3f(0,0,1);
						break;
					}

				glBegin(GL_QUADS);
					glVertex2f(v[GA(VF,0)]-POINT_SIZE, v[GA(VF,1)]+POINT_SIZE);
					glVertex2f(v[GA(VF,0)]+POINT_SIZE, v[GA(VF,1)]+POINT_SIZE);
					glVertex2f(v[GA(VF,0)]+POINT_SIZE, v[GA(VF,1)]-POINT_SIZE);
					glVertex2f(v[GA(VF,0)]-POINT_SIZE, v[GA(VF,1)]-POINT_SIZE);
				glEnd();
			}
		}

		float	p1x, p1y,
				p2x, p2y,
				p3x, p3y,
				p4x, p4y;

		p1x = br.start[GA(VF, 0)];
		p1y = br.start[GA(VF, 1)];
		p3x = br.end[GA(VF, 0)];
		p3y = br.end[GA(VF, 1)];

		p2x = p3x;
		p2y = p1y;
		p4x = p1x;
		p4y = p3y;

		Transform2DToPolar(&p1x, &p1y, 1);
		Transform2DToPolar(&p2x, &p2y, 1);
		Transform2DToPolar(&p3x, &p3y, 1);
		Transform2DToPolar(&p4x, &p4y, 1);

		glColor3f(1,0,0);

		glBegin(GL_LINES);
			glVertex2f(p1x,p1y);
			glVertex2f(p2x,p2y);
			glVertex2f(p2x,p2y);
			glVertex2f(p3x,p3y);
			glVertex2f(p3x,p3y);
			glVertex2f(p4x,p4y);
			glVertex2f(p4x,p4y);
			glVertex2f(p1x,p1y);
		glEnd();
		
		glDisable(GL_LINE_STIPPLE);

		if (rotate) {			
			float		v[3];
			float		x,y;
			CVector3	g = gpoint;

			v[_X] = g.x;
			v[_Y] = g.y;
			v[_Z] = g.z;

			x = v[GA(VF,0)];
			y = v[GA(VF,1)];

			Transform2DToPolar(&x,&y,1);

			glColor3f(0,0,1);
			glBegin(GL_QUADS);
				glVertex2f(x-2*POINT_SIZE, y-2*POINT_SIZE);
				glVertex2f(x+2*POINT_SIZE, y-2*POINT_SIZE);
				glVertex2f(x+2*POINT_SIZE, y+2*POINT_SIZE);
				glVertex2f(x-2*POINT_SIZE, y+2*POINT_SIZE);
			glEnd();
		}
	} else {
		int	width = LOWORD(*((LPARAM *)glwindow.GetInfo(GI_CSIZE,ONEDWINDOW)));
		
		float z1, z2;
		
		z1 = br.start[VF];
		z2 = br.end[VF];

		Transform1DToPolar(&z1, 1);
		Transform1DToPolar(&z2, 1);

		glColor3f(1,0,0);
		glBegin(GL_LINES);
			glVertex2f(0,z1);
			glVertex2f(width,z1);
			glVertex2f(0,z2);
			glVertex2f(width,z2);
		glEnd();
	}
}

void EDITOR::DrawSelectionMark3D(OBJECT obj, BRUSH br) {
	if (!br.active) return;

	BOX box;
	OBJECT tmp;

	glDisable(GL_TEXTURE_2D);

	if (dragvertices) {
		CVector3	*vbuff;
		float		v[3];
		int			num;

		vbuff = obj.GetControlPointList();
		num = obj.GetNumControlPoints();

		for (int i=0; i<num; i++) {
			v[_X] = vbuff[i].x;
			v[_Y] = vbuff[i].y;
			v[_Z] = vbuff[i].z;

			Transform2DToPolar(&v[GA(VF,0)], &v[GA(VF,1)], 1);

			glColor3f(1,0,0);

			for (int jj=0; jj<numselected; jj++)
				if (selected[jj][0]==br.object && selected[jj][1]==i) {
					glColor3f(0,0,1);
					break;
				}
			
			box.Init(CVector3(vbuff[i].x-POINT_SIZE,vbuff[i].y-POINT_SIZE,vbuff[i].z-POINT_SIZE), 
					 CVector3(vbuff[i].x+POINT_SIZE,vbuff[i].y+POINT_SIZE,vbuff[i].z+POINT_SIZE));
			box.GenerateShape();
			tmp.SetData(box);
			DrawObject3D(tmp);
			box.FreeMemory();
			tmp.FreeMemory();
		}
	}

	box.Init(CVector3(br.start[0], br.start[1], br.start[2]), 
			 CVector3(br.end[0], br.end[1], br.end[2]));
	box.GenerateShape();
	tmp.SetData(box);

	glColor4f(0.0, 0.0 ,1 , 0.9f);

	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	
	DrawObject3D(tmp, false);

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);
	
	tmp.FreeMemory();
	box.FreeMemory();
}


void EDITOR::Update() {
	int		tmp_idx;
	float	ROTATION_FACTOR		=	5.0f/gridSize;
	float	SCALE_FACTOR		=   1.0f/gridSize;

	Check_vCtr_Change();
	SetFocus(*(HWND*)glwindow.GetInfo(GI_HWND));
	
	DrawProjection();
	DrawModell();
	DrawOneD();
	DrawTexture();

	switch (designstatus) {
		case DS_NONE: {
			CVector2	pos2D;
			bool		pushed2D;

			pushed2D = *(bool*)glwindow.GetInfo(GI_CMOUSELB, PROJECTIONWINDOW);

			pos2D = GetMousePos2D();
			tmpbrush.end[GA(VF, 0)] = pos2D.x;
			tmpbrush.end[GA(VF, 1)] = pos2D.y;
			tmpbrush.end[VF] = DEFAULT_HEIGHT;
			tmpbrush.active = button2D;

			glwindow.PrintStatus("(%1.0f,%1.0f)", pos2D.x, pos2D.y);

			if (pushed2D && !button2D) {
				tmpbrush.start[GA(VF, 0)] = pos2D.x;
				tmpbrush.start[GA(VF, 1)] = pos2D.y;
				tmpbrush.start[VF] = 0;
				
				button2D = true;
			} else if (!pushed2D && button2D) {
				button2D = false;

				if (tmpbrush.start[_X]!=tmpbrush.end[_X] && 
					tmpbrush.start[_Y]!=tmpbrush.end[_Y]) {					

					tmpbrush.end[VF] = DEFAULT_HEIGHT;
					
					CreateObject(tmpobject, ST_BOX);

					/////////////////////////////
					Actualize();
					/////////////////////////////
				}
			} else if (button2D) {
				CreateObject(tmpobject, ST_BOX);
			}
			break;
		}
		case DS_MODIFY_OBJECTS: {
			float		v[3];
			float		t[3];
			float       pos1D;
			CVector2	pos2D;
			bool		pushed1D,
						pushed2D;

			pushed1D = *(bool*)glwindow.GetInfo(GI_CMOUSELB, ONEDWINDOW);
			pushed2D = *(bool*)glwindow.GetInfo(GI_CMOUSELB, PROJECTIONWINDOW);
			
			// processing 2D transform
			if (pushed2D && !button2D) {
				pos2D = GetMousePos2D(false);
				v[GA(VF,0)] = pos2D.x;
				v[GA(VF,1)] = pos2D.y;
				v[VF] = 0;

				dragall = InsideBrush2D(tmpbrush, v);

				for (int i=0; i<numbrushes; i++)
					if (InsideBrush2D(brush[i], v)) {
						dragall = true;
						break;
					}

				pos2D = GetMousePos2D();
				v[GA(VF,0)] = pos2D.x;
				v[GA(VF,1)] = pos2D.y;
				v[VF] = 0;
				
				clickpos2D = CVector3(v[_X],v[_Y],v[_Z]);

				button2D = true;
			} else if (!pushed2D && button2D) {
				button2D = false;
			} else if (button2D) { // modifying objects while dragging

				pos2D = GetMousePos2D();	// asking for the new mousepos
				v[GA(VF,0)] = pos2D.x;
				v[GA(VF,1)] = pos2D.y;
				v[VF] = 0;

				t[_X] = clickpos2D.x;
				t[_Y] = clickpos2D.y;
				t[_Z] = clickpos2D.z;

				
				// initializing coordonates. Needed to calculate dragg length
				if (dragall || !AreZeroSizedObjects2D()) {
					// moving/resizing all the brushes, and objects
					for (int i=0; i<numbrushes; i++)
						ModifyBrush2D(brush[i], object[brush[i].object], t,v, dragall);

					ModifyBrush2D(tmpbrush, tmpobject, t,v, dragall);
				}				

				for (int i=0;i<numbulbsel; i++)
					bulb[bulb_sel[i]].pos = bulb[bulb_sel[i]].pos + CVector3(v[0]-t[0],v[1]-t[1],v[2]-t[2]);
				

				// moving mouse to the last processed position
				clickpos2D = CVector3(v[_X],v[_Y],v[_Z]); 
			}
			

			// processing 1D transform
			if (pushed1D && !button1D) {
				pos1D = GetMousePos1D(false);
				v[GA(VF,0)] = 0;
				v[GA(VF,1)] = 0;
				v[VF] = pos1D;

				dragall = InsideBrush1D(tmpbrush, v);

				for (int i=0; i<numbrushes; i++)
					if (InsideBrush1D(brush[i], v)) {
						dragall = true;
						break;
					}

				pos1D = GetMousePos1D();
				v[GA(VF,0)] = 0;
				v[GA(VF,1)] = 0;
				v[VF] = pos1D;
				
				clickpos1D = CVector3(v[_X],v[_Y],v[_Z]);

				button1D = true;
			} else if (!pushed1D && button1D) {
				button1D = false;
			} else if (button1D) { // modifying objects while dragging

				pos1D = GetMousePos1D();	// asking for the new mousepos
				v[GA(VF,0)] = 0;
				v[GA(VF,1)] = 0;
				v[VF] = pos1D;

				// initializing coordonates. Needed to calculate dragg length
				t[_X] = clickpos1D.x;
				t[_Y] = clickpos1D.y;
				t[_Z] = clickpos1D.z;

				
				if (dragall || !AreZeroSizedObjects1D()) {
					// moving/resizing all the brushes, and objects
					for (int i=0; i<numbrushes; i++)
						ModifyBrush1D(brush[i], object[brush[i].object], t,v, dragall);
					ModifyBrush1D(tmpbrush, tmpobject, t,v, dragall);
				}

				for (int i=0;i<numbulbsel; i++)
					bulb[bulb_sel[i]].pos = bulb[bulb_sel[i]].pos + CVector3(v[0]-t[0],v[1]-t[1],v[2]-t[2]);

				// moving mouse to the last processed position
				clickpos1D = CVector3(v[_X],v[_Y],v[_Z]); 				
			}

			break;
		}
		case DS_SCALE_OBJECTS: {
			float		v;
			float		t;
			CVector2	pos2D;
			bool		pushed2D;

			pushed2D = *(bool*)glwindow.GetInfo(GI_CMOUSELB, PROJECTIONWINDOW);
			
			// processing 2D transform
			if (pushed2D && !button2D) {
				pos2D = GetMousePos2D(false);
				v = pos2D.y;
				clickpos2D = CVector3(0, v, 0);

				button2D = true;
			} else if (!pushed2D && button2D) {
				button2D = false;
			} else if (button2D) { // modifying objects while dragging

				pos2D = GetMousePos2D(false);	// asking for the new mousepos
				v = pos2D.y;

				t = clickpos2D.y;

				for (int i=0; i<numbrushes; i++)
					ScaleBrush(brush[i], object[brush[i].object], (t-v));
				ScaleBrush(tmpbrush, tmpobject, (t-v));

				// moving mouse to the last processed position
				clickpos2D = CVector3(0 , v, 0); 
			}
			break;
		}
		case DS_SELECT_CONTROLS: {
			CVector2	pos2D;
			bool		pushed2D;

			pushed2D = *(bool*)glwindow.GetInfo(GI_CMOUSELB, PROJECTIONWINDOW);

			pos2D = GetMousePos2D(false);
			sel_end[0] = pos2D.x;
			sel_end[1] = pos2D.y;
			
			glwindow.PrintStatus("(%1.0f,%1.0f) - (%1.0f,%1.0f)",
								 sel_start[0],sel_start[1],sel_end[0],sel_end[1]);

			if (pushed2D && !button2D) {
				sel_start[0] = pos2D.x;
				sel_start[1] = pos2D.y;
				
				numselected = 0;		
				activeselection = true;

				button2D = true;
			} else if (!pushed2D && button2D) {
				button2D = false;
				activeselection = false;

				CVector3	*vbuff;
				int			num;

				for (int i=0; i<numbrushes; i++) {
					vbuff = object[brush[i].object].GetControlPointList();
					num = object[brush[i].object].GetNumControlPoints();
					for (int jj=0; jj<num; jj++) {
						if (InsideSelection(vbuff[jj]) && numselected<MAX_SELECTED) {							
							selected[numselected][0]=brush[i].object;
							selected[numselected][1]=jj;

							numselected++;
						}
					}
				}
				

				if (tmpobject.IsInitialized()) {
					vbuff = tmpobject.GetControlPointList();
					num = tmpobject.GetNumControlPoints();
					for (int jj=0; jj<num; jj++) {
						if (InsideSelection(vbuff[jj]) && numselected<MAX_SELECTED) {
							selected[numselected][0]=NONE;
							selected[numselected][1]=jj;
							numselected++;
						}
					}
				}


				//////////////////
				Actualize();
				//////////////////
			} 
			break;			
		}
		case DS_MODIFY_CONTROLS: {
			float		v[3];
			float		t[3];
			CVector2	pos2D;
			bool		pushed2D;
			CVector3	*point;

			pushed2D = *(bool*)glwindow.GetInfo(GI_CMOUSELB, PROJECTIONWINDOW);
			
			// processing 2D transform
			if (pushed2D && !button2D) {
				pos2D = GetMousePos2D(false);
				v[GA(VF,0)] = pos2D.x;
				v[GA(VF,1)] = pos2D.y;
				v[VF] = 0;
				clickpos2D = CVector3(v[_X],v[_Y],v[_Z]);

				dragall = false;
				for (int i=0; i<numselected; i++) {
					if (selected[i][0]==-1) point = &tmpobject.GetControlPointList()[selected[i][1]];
					else point = &object[selected[i][0]].GetControlPointList()[selected[i][1]];

					if (HitVertex(*point, clickpos2D)) {
						dragall = true;
						break;
					}
				}

				pos2D = GetMousePos2D();
				v[GA(VF,0)] = pos2D.x;
				v[GA(VF,1)] = pos2D.y;
				v[VF] = 0;
				clickpos2D = CVector3(v[_X],v[_Y],v[_Z]);

				button2D = true;
			} else if (!pushed2D && button2D) {
				button2D = false;
			} else if (button2D) { // modifying objects while dragging

				pos2D = GetMousePos2D();	// asking for the new mousepos
				v[GA(VF,0)] = pos2D.x;
				v[GA(VF,1)] = pos2D.y;
				v[VF] = 0;

				// initializing coordonates. Needed to calculate dragg length
				t[_X] = clickpos2D.x;
				t[_Y] = clickpos2D.y;
				t[_Z] = clickpos2D.z;

				if (!dragall) BackStatus();

				if (dragall) {
					for (int i=0; i<numselected; i++) {
						if (selected[i][0]==-1) point = &tmpobject.GetControlPointList()[selected[i][1]];
						else point = &object[selected[i][0]].GetControlPointList()[selected[i][1]];

						(*point) = (*point) - CVector3(t[_X]-v[_X],t[_Y]-v[_Y],t[_Z]-v[_Z]);
					}
					for (i=0; i<numbrushes; i++) {
						Recalc(object[brush[i].object]);
						tmp_idx = brush[i].object;
						brush[i] = object[brush[i].object].GetBBoxO();
						brush[i].object = tmp_idx;
					}
					if (tmpbrush.active) {
						Recalc(tmpobject);
						tmpbrush = tmpobject.GetBBoxO();
					}	
					
					clickpos2D = CVector3(v[_X],v[_Y],v[_Z]); 
				}			
			}
			break;
		}
		case DS_ROTATE_OBJECTS: {
			float		v;
			float		t;
			CVector2	pos2D;
			bool		pushed2D;

			pushed2D = *(bool*)glwindow.GetInfo(GI_CMOUSELB, PROJECTIONWINDOW);
			
			// processing 2D transform
			if (pushed2D && !button2D) {
				pos2D = GetMousePos2D();				
				v = pos2D.y;
				clickpos2D = CVector3(0,v,0);
				button2D = true;
			} else if (!pushed2D && button2D) {
				button2D = false;
			} else if (button2D) { // modifying objects while dragging
				pos2D = GetMousePos2D();	// asking for the new mousepos
				v = pos2D.y;

				t = clickpos2D.y;
				gpoint = GetHub();

				for (int i=0; i<numbrushes; i++) {
					RotateObject(object[brush[i].object], ROTATION_FACTOR*(t-v), gpoint);

					tmp_idx = brush[i].object;
					brush[i] = object[brush[i].object].GetBBoxO();
					brush[i].object = tmp_idx;
				}

				if (tmpobject.IsInitialized()) {
					RotateObject(tmpobject, ROTATION_FACTOR*(t-v), gpoint);
					tmpbrush = tmpobject.GetBBoxO();
				}

				// moving mouse to the last processed position
				clickpos2D = CVector3(0,v,0); 
			}			
			break;
		}
		case DS_ROTATE_CONTROLS: {
			float		v;
			float		t;
			CVector2	pos2D;
			bool		pushed2D;

			pushed2D = *(bool*)glwindow.GetInfo(GI_CMOUSELB, PROJECTIONWINDOW);
			
			// processing 2D transform
			if (pushed2D && !button2D) {
				pos2D = GetMousePos2D();				
				v = pos2D.y;
				clickpos2D = CVector3(0,v,0);
				button2D = true;				
			} else if (!pushed2D && button2D) {
				button2D = false;
			} else if (button2D) { // modifying objects while dragging
				pos2D = GetMousePos2D();	// asking for the new mousepos
				v = pos2D.y;

				t = clickpos2D.y;
				
				CVector3	*point;
				
				for (int i=0; i<numselected; i++) {
					if (selected[i][0]==-1) point = &tmpobject.GetControlPointList()[selected[i][1]];
					else point = &object[selected[i][0]].GetControlPointList()[selected[i][1]];

					RotatePoint(point, ROTATION_FACTOR*(t-v), gpoint);
				}

				for (i=0; i<numbrushes; i++) {
					Recalc(object[brush[i].object]);
					tmp_idx = brush[i].object;
					brush[i] = object[brush[i].object].GetBBoxO();
					brush[i].object = tmp_idx;
				}

				if (tmpobject.IsInitialized()) {
					Recalc(tmpobject);
					tmpbrush = tmpobject.GetBBoxO();
				}

				// moving mouse to the last processed position
				clickpos2D = CVector3(0,v,0); 
			}

			break;
		}
	}

	POINT turn;
	turn = *(POINT *)glwindow.GetInfo(GI_CRDRAG, MODELLWINDOW);
	glwindow.SetInfo(SI_CRDRAG, NULL, MODELLWINDOW);
	camera.TurnTo(turn.x, turn.y);
}

bool EDITOR::AreZeroSizedObjects1D() {
	float		s[3];
	float		e[3];
	BRUSH		b;

	for (int i=0; i<numbrushes; i++) {
		b = object[brush[i].object].GetBBoxN(false);

		s[_X] = b.start[_X];
		s[_Y] = b.start[_Y];
		s[_Z] = b.start[_Z];
		e[_X] = b.end[_X];
		e[_Y] = b.end[_Y];
		e[_Z] = b.end[_Z];

		if (fabs(s[VF]-e[VF])<NEAR_ZERO)
				return true;
	}

	if (tmpobject.IsInitialized()) {
		b = tmpobject.GetBBoxN(false);

		s[_X] = b.start[_X];
		s[_Y] = b.start[_Y];
		s[_Z] = b.start[_Z];
		e[_X] = b.end[_X];
		e[_Y] = b.end[_Y];
		e[_Z] = b.end[_Z];

		if (fabs(s[VF]-e[VF])<NEAR_ZERO)
				return true;
	}

	return false;
}


bool EDITOR::GetLightInfo(BULB &b) {
	if (numbulbsel!=1) return false;
	
	b = bulb[bulb_sel[0]];
	
	return true;
}

void EDITOR::SetLightInfo(BULB b) {
	if (numbulbsel!=1) return;
	
	bulb[bulb_sel[0]]=b;
}


bool EDITOR::GetTextureInfo(CVector2 &size, CVector2 &shift, float &tr, bool &bb) {
	if (numbrushes>0 || !tmpbrush.active) return false;
	
	size = tmpobject.GetTextureSize();
	shift = tmpobject.GetTextureShift();
	tr = tmpobject.GetPTextureInfo()->transparency;
	bb = tmpobject.GetPTextureInfo()->billboarding;
	
	return true;
}

void EDITOR::SetTextureInfo(CVector2 size, CVector2 shift, float tr, bool bb) {
	if (numbrushes>0 || !tmpbrush.active) return;
	
	tmpobject.GetPTextureInfo()->transparency = tr;
	tmpobject.GetPTextureInfo()->billboarding = bb;
	tmpobject.tex_shift = shift;
	tmpobject.tex_size = size;	
}


bool EDITOR::AreZeroSizedObjects2D() {
	float		s[3];
	float		e[3];
	BRUSH		b;

	for (int i=0; i<numbrushes; i++) {
		b = object[brush[i].object].GetBBoxN(false);

		s[_X] = b.start[_X];
		s[_Y] = b.start[_Y];
		s[_Z] = b.start[_Z];
		e[_X] = b.end[_X];
		e[_Y] = b.end[_Y];
		e[_Z] = b.end[_Z];

		if (fabs(s[GA(VF,0)]-e[GA(VF,0)])<NEAR_ZERO ||
			fabs(s[GA(VF,1)]-e[GA(VF,1)])<NEAR_ZERO)
				return true;
	}

	if (tmpobject.IsInitialized()) {
		b = tmpobject.GetBBoxN(false);

		s[_X] = b.start[_X];
		s[_Y] = b.start[_Y];
		s[_Z] = b.start[_Z];
		e[_X] = b.end[_X];
		e[_Y] = b.end[_Y];
		e[_Z] = b.end[_Z];

		if (fabs(s[GA(VF,0)]-e[GA(VF,0)])<NEAR_ZERO ||
			fabs(s[GA(VF,1)]-e[GA(VF,1)])<NEAR_ZERO)
				return true;
	}

	return false;
}


void EDITOR::RotatePoint(CVector3 *point, float deg, CVector3 centrum) {
	float		p[3];

	p[_X] = point->x - centrum.x;
	p[_Y] = point->y - centrum.y;
	p[_Z] = point->z - centrum.z;

	rot(deg, p[GA(VF,0)], p[GA(VF,1)]);

	(*point) = CVector3(centrum.x + p[_X], centrum.y + p[_Y], centrum.z + p[_Z]);
}


void EDITOR::RotateObject(OBJECT &obj, float deg, CVector3 centrum) {
	if (!obj.IsInitialized()) return;

	float		p[3];
	CVector3	*vbuff;
	int			num;
	

	vbuff = obj.GetVertexList();
	num = obj.GetNumPoints();

	for (int i=0; i<num; i++) {
		p[_X] = vbuff[i].x - centrum.x;
		p[_Y] = vbuff[i].y - centrum.y;
		p[_Z] = vbuff[i].z - centrum.z;
		
		rot(deg, p[GA(VF,0)], p[GA(VF,1)]);

		vbuff[i] = CVector3(centrum.x + p[_X], centrum.y + p[_Y], centrum.z + p[_Z]);
	}
	
	vbuff = obj.GetControlPointList();
	num = obj.GetNumControlPoints();

	for (i=0; i<num; i++) {
		p[_X] = vbuff[i].x - centrum.x;
		p[_Y] = vbuff[i].y - centrum.y;
		p[_Z] = vbuff[i].z - centrum.z;
		
		rot(deg, p[GA(VF,0)], p[GA(VF,1)]);
				
		vbuff[i] = CVector3(centrum.x + p[_X], centrum.y + p[_Y], centrum.z + p[_Z]);
	}

	if (!locktexrotation) obj.FitTexture();
}


CVector3 EDITOR::GetHub() {	
	CVector3	center;
	int			num = numbrushes;

	for (int i=0; i<numbrushes; i++) {
		center = center + object[brush[i].object].GetCenter();
	}
	
	if (tmpobject.IsInitialized()) {
		num++;
		center = center + tmpobject.GetCenter();
	}
	return num>0 ? (center/num):center;
}


bool EDITOR::HitVertex(CVector3 point, CVector3 tt) {
	float v[3];
	float t[3];

	CVector2 p1, p2;

	v[_X] = point.x;
	v[_Y] = point.y;
	v[_Z] = point.z;
	t[_X] = tt.x;
	t[_Y] = tt.y;
	t[_Z] = tt.z;

	p1 = CVector2(v[GA(VF,0)],v[GA(VF,1)]);
	p2 = CVector2(t[GA(VF,0)],t[GA(VF,1)]);

	return Distance(p1, p2) < 2*POINT_SIZE;
}

bool EDITOR::IsPointInBox2D(CVector3 pos, BRUSH sel) {
	float v[3];
	int sg0 = SGN(sel.end[GA(VF,0)] - sel.start[GA(VF,0)]);
	int sg1 = SGN(sel.end[GA(VF,1)] - sel.start[GA(VF,1)]);

	v[0] = pos.x;
	v[1] = pos.y;
	v[2] = pos.z;
	
	return	sg0*sel.start[GA(VF,0)] <= sg0*v[GA(VF,0)]	&& sg0*v[GA(VF,0)]	<= sg0*sel.end[GA(VF,0)] &&
			sg1*sel.start[GA(VF,1)] <= sg1*v[GA(VF,1)]	&& sg1*v[GA(VF,1)]	<= sg1*sel.end[GA(VF,1)];
}


bool EDITOR::IsBoxInBox2D(BRUSH box, BRUSH sel) {
	int sg0 = SGN(sel.end[GA(VF,0)] - sel.start[GA(VF,0)]);
	int sg1 = SGN(sel.end[GA(VF,1)] - sel.start[GA(VF,1)]);
	
	return	sg0*sel.start[GA(VF,0)] <= sg0*box.start[GA(VF,0)]	&& sg0*box.start[GA(VF,0)]	<= sg0*sel.end[GA(VF,0)] &&
			sg0*sel.start[GA(VF,0)] <= sg0*box.end[GA(VF,0)]	&& sg0*box.end[GA(VF,0)]	<= sg0*sel.end[GA(VF,0)] &&
			sg1*sel.start[GA(VF,1)] <= sg1*box.start[GA(VF,1)]	&& sg1*box.start[GA(VF,1)]	<= sg1*sel.end[GA(VF,1)] &&
			sg1*sel.start[GA(VF,1)] <= sg1*box.end[GA(VF,1)]	&& sg1*box.end[GA(VF,1)]	<= sg1*sel.end[GA(VF,1)];
}


bool EDITOR::InsideSelection(CVector3 p) {
	int sg0 = SGN(sel_end[0] - sel_start[0]);
	int sg1 = SGN(sel_end[1] - sel_start[1]);

	float v[3];

	v[_X] = p.x;
	v[_Y] = p.y;
	v[_Z] = p.z;

	return	sel_start[0]*sg0 <= v[GA(VF,0)]*sg0 && v[GA(VF,0)]*sg0<=sel_end[0]*sg0 &&
			sel_start[1]*sg1 <= v[GA(VF,1)]*sg1 && v[GA(VF,1)]*sg1<=sel_end[1]*sg1;
}


bool EDITOR::InsideBrush2D(BRUSH b, float v[3]) {
	if (!b.active) return false;

	int sg0 = SGN(b.end[GA(VF,0)] - b.start[GA(VF,0)]);
	int sg1 = SGN(b.end[GA(VF,1)] - b.start[GA(VF,1)]);

	return	b.start[GA(VF,0)]*sg0<v[GA(VF,0)]*sg0 && v[GA(VF,0)]*sg0<b.end[GA(VF,0)]*sg0 &&
			b.start[GA(VF,1)]*sg1<v[GA(VF,1)]*sg1 && v[GA(VF,1)]*sg1<b.end[GA(VF,1)]*sg1;
}

bool EDITOR::InsideBrush1D(BRUSH b, float v[3]) {
	if (!b.active) return false;

	int sg = SGN(b.end[VF] - b.start[VF]);

	return	b.start[VF]*sg<v[VF]*sg && v[VF]*sg<b.end[VF]*sg;			
}

bool EDITOR::InitWindows() {
	HWND		chWnd[NUM_CHILD_WINDOWS];
	WNDCLASSEX	cwc[NUM_CHILD_WINDOWS-1];
	HINSTANCE   hInstance;
	UINT		styles[NUM_CHILD_WINDOWS];
	char	   *className[NUM_CHILD_WINDOWS];
	
	RECT		rc;
	int			width, 
				height;


	// initializing styles for the windows
	styles[PROJECTIONWINDOW] = 
	styles[MODELLWINDOW]	 =
	styles[ONEDWINDOW]		 = WS_THICKFRAME | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS;
	styles[TEXTUREWINDOW]	 = WS_THICKFRAME | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
							   WS_VSCROLL;
	styles[LOGWINDOW]		 = WS_THICKFRAME | WS_CHILD | WS_CLIPCHILDREN | WS_CLIPSIBLINGS |
							   ES_AUTOHSCROLL | ES_AUTOVSCROLL | WS_VSCROLL | WS_HSCROLL |
							   ES_MULTILINE | ES_NOHIDESEL;

	// initializing class names
	className[PROJECTIONWINDOW] = "Projection View";
	className[MODELLWINDOW]		= "Modell View";
	className[TEXTUREWINDOW]	= "Texture View Window";
	className[ONEDWINDOW]		= "1D View Window";
	className[LOGWINDOW]		= "EDIT";


	glwindow.Init(640, 480, 32, false, "Editor", 0,(WNDPROC) WndProc, IDR_MENU);

	hInstance = *((HINSTANCE *)glwindow.GetInfo(GI_HINSTANCE));


	HWND hWnd = *((HWND *) glwindow.GetInfo(GI_HWND));
	GetClientRect(hWnd, &rc);

	width = rc.right - rc.left;
	height = rc.bottom - rc.top - STATUS_HEIGHT;
		
	for (int i=0; i<NUM_CHILD_WINDOWS-1; i++) {
		memset(&cwc[i], 0, sizeof(WNDCLASSEX));

		cwc[i].cbSize		= sizeof(WNDCLASSEX);
		cwc[i].style		= CS_HREDRAW | CS_VREDRAW;
		cwc[i].cbClsExtra	= 0;
		cwc[i].cbWndExtra	= 0;
		cwc[i].hInstance	= hInstance;
		cwc[i].hCursor		= LoadCursor(NULL, IDC_ARROW);
		cwc[i].hbrBackground= (HBRUSH)GetStockObject(WHITE_BRUSH);
		cwc[i].lpszClassName= className[i];
	}

	cwc[PROJECTIONWINDOW].lpfnWndProc	= (WNDPROC)WP_ProjectWin;
	cwc[MODELLWINDOW].lpfnWndProc		= (WNDPROC)WP_ModellWin;
	cwc[TEXTUREWINDOW].lpfnWndProc		= (WNDPROC)WP_TextureWin;
	cwc[ONEDWINDOW].lpfnWndProc			= (WNDPROC)WP_OneDWin;
	cwc[LOGWINDOW].lpfnWndProc			= (WNDPROC)WndProc;

	// NUM_CHILD_WINDOWS-1 because the last window already has a predefined class
	for (i=0; i<NUM_CHILD_WINDOWS-1; i++) {
		RegisterClassEx(&cwc[i]);
	}

	for (i=0; i<NUM_CHILD_WINDOWS; i++)
	{
		chWnd[i] = CreateWindowEx(WS_EX_OVERLAPPEDWINDOW,
								  className[i],
								  className[i],
								  styles[i],
								  (int)(g_rect[i].x*width),
								  (int)(g_rect[i].y*height),
								  (int)(g_rect[i].width*width),
								  (int)(g_rect[i].height*height),
								  hWnd,
								  (HMENU) (FIRST_CHILD_WINDOW_ID + i),
								  GetModuleHandle(NULL),
								  NULL);
		if (chWnd[i] == NULL) {
			return false;
		}

		ShowWindow(chWnd[i], SW_SHOWNORMAL);
		UpdateWindow(chWnd[i]);
	}	

	
	glwindow.AddChildWindows(chWnd, className, NUM_CHILD_WINDOWS);

	SetWindowText(chWnd[LOGWINDOW], "Application Started.\r\n ");
	
	
	glwindow.InitializeOpenGLType(PROJECTIONWINDOW);
	glwindow.InitializeOpenGLType(MODELLWINDOW);
	glwindow.InitializeOpenGLType(TEXTUREWINDOW);
	glwindow.InitializeOpenGLType(ONEDWINDOW);

	return true;
}


void EDITOR::ModifyBrush1D(BRUSH &br, OBJECT &obj, float t[3], float v[3], bool drag) {
	if (!br.active) return;
	if (!obj.IsInitialized()) return;

	int		osg, nsg;
	BRUSH	ob = br;		// saving the brush	

	if (drag) {
		obj.Drag(CVector3(v[_X]-t[_X],
						  v[_Y]-t[_Y],
						  v[_Z]-t[_Z]));

		br.start[VF] += v[VF]-t[VF];
		br.end[VF] += v[VF]-t[VF];
	} else {
		osg = SGN(br.end[VF] - br.start[VF]);

		if (t[VF]*osg <= br.start[VF]*osg)
			br.start[VF] += v[VF]-t[VF];

		if (t[VF]*osg >= br.end[VF]*osg)
			br.end[VF] += v[VF]-t[VF];

		nsg = SGN(br.end[VF] - br.start[VF]);

		if (nsg!=osg || fabs(br.end[VF] - br.start[VF])<MIN_SIZE) {
			br.start[VF] = ob.start[VF];
			br.end[VF] = ob.end[VF];
		}
	
			
		FitToBrush(obj, br, ob);
	}
}


void EDITOR::ModifyBrush2D(BRUSH &br, OBJECT &obj, float t[3], float v[3], bool drag) {
	if (!br.active) return;
	if (!obj.IsInitialized()) return;

	int		osg0, nsg0;
	int		osg1, nsg1;
	BRUSH	ob = br;		// saving brush


	if (drag) {
		obj.Drag(CVector3(v[_X]-t[_X],
						  v[_Y]-t[_Y],
						  v[_Z]-t[_Z]));
		br.start[GA(VF,0)] += v[GA(VF,0)]-t[GA(VF,0)];
		br.start[GA(VF,1)] += v[GA(VF,1)]-t[GA(VF,1)];
		br.end[GA(VF,0)] += v[GA(VF,0)]-t[GA(VF,0)];
		br.end[GA(VF,1)] += v[GA(VF,1)]-t[GA(VF,1)];
	} else {
		

		osg0 = SGN(br.end[GA(VF,0)] - br.start[GA(VF,0)]);
		osg1 = SGN(br.end[GA(VF,1)] - br.start[GA(VF,1)]);

		if (t[GA(VF,0)]*osg0 <= br.start[GA(VF,0)]*osg0)
			br.start[GA(VF,0)] += v[GA(VF,0)]-t[GA(VF,0)];

		if (t[GA(VF,1)]*osg1 <= br.start[GA(VF,1)]*osg1)
			br.start[GA(VF,1)] += v[GA(VF,1)]-t[GA(VF,1)];

		if (t[GA(VF,0)]*osg0 >= br.end[GA(VF,0)]*osg0)
			br.end[GA(VF,0)] += v[GA(VF,0)]-t[GA(VF,0)];

		if (t[GA(VF,1)]*osg1 >= br.end[GA(VF,1)]*osg1)
			br.end[GA(VF,1)] += v[GA(VF,1)]-t[GA(VF,1)];

		nsg0 = SGN(br.end[GA(VF,0)] - br.start[GA(VF,0)]);
		nsg1 = SGN(br.end[GA(VF,1)] - br.start[GA(VF,1)]);

		if (nsg0!=osg0 || fabs(br.end[GA(VF,0)] - br.start[GA(VF,0)])<MIN_SIZE) {
			br.start[GA(VF,0)] = ob.start[GA(VF,0)];
			br.end[GA(VF,0)] = ob.end[GA(VF,0)];
		}
		if (nsg1!=osg1 || fabs(br.end[GA(VF,1)] - br.start[GA(VF,1)])<MIN_SIZE) {
			br.start[GA(VF,1)] = ob.start[GA(VF,1)];
			br.end[GA(VF,1)] = ob.end[GA(VF,1)];
		}
			
		FitToBrush(obj, br, ob);
	}	
}


void EDITOR::ScaleBrush(BRUSH &br, OBJECT &obj, float coef) {
	if (!br.active) return;
	if (!obj.IsInitialized()) return;

	if (fabs(coef)<1) return;

	coef = 1.0f + 0.1f * (coef<0.0f ? -1.0f:1.0f);

	CVector3 start, end;
	start = CVector3(br.start[_X], br.start[_Y], br.start[_Z]);
	end = CVector3(br.end[_X], br.end[_Y], br.end[_Z]);

	start = gpoint + (start - gpoint) * coef;
	end = gpoint + (end - gpoint) * coef;

	br.start[_X] = start.x; 
	br.start[_Y] = start.y; 
	br.start[_Z] = start.z; 
	br.end[_X] = end.x; 
	br.end[_Y] = end.y; 
	br.end[_Z] = end.z; 

	int			numpoints;
	CVector3	*vbuff;
		
	numpoints = obj.GetNumPoints();
	vbuff = obj.GetVertexList();

	for (int i=0; i<numpoints; i++)
		vbuff[i] = gpoint + (vbuff[i] - gpoint) * coef;

	numpoints = obj.GetNumControlPoints();
	vbuff = obj.GetControlPointList();

	for (i=0; i<numpoints; i++)
		vbuff[i] = gpoint + (vbuff[i] - gpoint) * coef;

	if (!locktexmove) obj.FitTexture();
}


void EDITOR::FitToBrush(OBJECT &obj, BRUSH br, BRUSH ob) {
	int			numpoints;
	CVector3	*vbuff;
		
	numpoints = obj.GetNumPoints();
	vbuff = obj.GetVertexList();

	for (int i=0; i<numpoints; i++) {
		if (fabs(ob.end[_X]-ob.start[_X])>NEAR_ZERO) 
			vbuff[i].x = br.start[_X] + (br.end[_X]-br.start[_X])*(vbuff[i].x-ob.start[_X])/(ob.end[_X]-ob.start[_X]);
		if (fabs(ob.end[_Y]-ob.start[_Y])>NEAR_ZERO) 
			vbuff[i].y = br.start[_Y] + (br.end[_Y]-br.start[_Y])*(vbuff[i].y-ob.start[_Y])/(ob.end[_Y]-ob.start[_Y]);
		if (fabs(ob.end[_Z]-ob.start[_Z])>NEAR_ZERO)
			vbuff[i].z = br.start[_Z] + (br.end[_Z]-br.start[_Z])*(vbuff[i].z-ob.start[_Z])/(ob.end[_Z]-ob.start[_Z]);
	}

	numpoints = obj.GetNumControlPoints();
	vbuff = obj.GetControlPointList();

	for (i=0; i<numpoints; i++) {
		if (fabs(ob.end[_X]-ob.start[_X])>NEAR_ZERO)
			vbuff[i].x = br.start[_X] + (br.end[_X]-br.start[_X])*(vbuff[i].x-ob.start[_X])/(ob.end[_X]-ob.start[_X]);
		if (fabs(ob.end[_Y]-ob.start[_Y])>NEAR_ZERO)
			vbuff[i].y = br.start[_Y] + (br.end[_Y]-br.start[_Y])*(vbuff[i].y-ob.start[_Y])/(ob.end[_Y]-ob.start[_Y]);
		if (fabs(ob.end[_Z]-ob.start[_Z])>NEAR_ZERO)
			vbuff[i].z = br.start[_Z] + (br.end[_Z]-br.start[_Z])*(vbuff[i].z-ob.start[_Z])/(ob.end[_Z]-ob.start[_Z]);
	}

	BRUSH tmp = obj.GetBBoxN();
	obj.SetCenter((tmp.GetStart()+tmp.GetEnd())/2);
	if (!locktexmove) obj.FitTexture();
}


void EDITOR::Recalc(OBJECT &obj) {
	if (!obj.IsInitialized()) return;

	BEVEL		bevel;
	BOX			box;
	ENDCAP		endcap;
	NCILINDER	ncilinder;
	NCONE		ncone;
	NPLANE		nplane;
	SPHERE		sphere;
	ECAP		ecap;
	BCAP		bcap;
	IECAP		iecap;
	IBCAP		ibcap;

	byte		orientation = obj.GetOrientation();

	switch(obj.GetType()) {
		case ST_BEVEL:
			bevel.SetSmoothness(obj.GetSmoothness());
			bevel.SetControlPoints(obj.GetControlPointList());
			bevel.GenerateShape();
			obj.SetData(bevel);
			break;
		case ST_BOX:
			box.SetSmoothness(obj.GetSmoothness());
			box.SetControlPoints(obj.GetControlPointList());
			box.GenerateShape();
			obj.SetData(box);
			break;
		case ST_ENDCAP:
			endcap.SetSmoothness(obj.GetSmoothness());
			endcap.SetControlPoints(obj.GetControlPointList());
			endcap.GenerateShape();
			obj.SetData(endcap);
			break;
		case ST_NCILINDER:
			
			ncilinder.SetSmoothness(obj.GetSmoothness());
			ncilinder.SetControlPoints(obj.GetControlPointList());
			ncilinder.GenerateShape();
			obj.SetData(ncilinder);
			
			break;
		case ST_NCONE:
			ncone.SetSmoothness(obj.GetSmoothness());
			ncone.SetControlPoints(obj.GetControlPointList());
			ncone.GenerateShape();
			obj.SetData(ncone);
			break;
		case ST_NPLANE:
			nplane.SetSmoothness(obj.GetSmoothness());
			nplane.SetControlPoints(obj.GetControlPointList());
			nplane.GenerateShape();
			obj.SetData(nplane);
			break;
		case ST_SPHERE:
			sphere.SetSmoothness(obj.GetSmoothness());
			sphere.SetControlPoints(obj.GetControlPointList());
			sphere.GenerateShape();
			obj.SetData(sphere);
			break;
		case ST_ECAP:
			ecap.SetSmoothness(obj.GetSmoothness());
			ecap.SetControlPoints(obj.GetControlPointList());
			ecap.GenerateShape();
			obj.SetData(ecap);
			break;
		case ST_BCAP:
			bcap.SetSmoothness(obj.GetSmoothness());
			bcap.SetControlPoints(obj.GetControlPointList());
			bcap.GenerateShape();
			obj.SetData(bcap);
			break;
		case ST_IECAP:
			iecap.SetSmoothness(obj.GetSmoothness());
			iecap.SetControlPoints(obj.GetControlPointList());
			iecap.GenerateShape();
			obj.SetData(iecap);
			break;
		case ST_IBCAP:
			ibcap.SetSmoothness(obj.GetSmoothness());
			ibcap.SetControlPoints(obj.GetControlPointList());
			ibcap.GenerateShape();
			obj.SetData(ibcap);
			break;
	}

	obj.SetOrientation(orientation);

	BRUSH tmp;
	
	tmp = obj.GetBBoxN();
	obj.SetCenter(CVector3((tmp.start[_X]+tmp.end[_X])/2,
				 		   (tmp.start[_Y]+tmp.end[_Y])/2,
				 		   (tmp.start[_Z]+tmp.end[_Z])/2));

	obj.FitTexture();
}


void EDITOR::SetViewedFrom(int VF) {
	this->VF = VF; 
	activeselection = false;
}

void EDITOR::InitEditor() 
{
	LPARAM point = *((LPARAM *)glwindow.GetInfo(GI_CCENTER, PROJECTIONWINDOW));
	
	vCtr[_X] = vCtr[_Y] = vCtr[_Z] = 0.0f;

	designstatus = DS_NONE;

	gridSize	= 8;
	mapSize		= 1024;
	VF			= FRONT;

	selectedtexture = NONE;
	activeselection = false;

	numbrushes	= 0;
	numobjects	= 0;
	numselected = 0;
	scalemap	= 1.0f;

	showbulbs		= true;
	showlightmaps	= true;

	dragvertices	= false;
	rotate			= false;

	SnapToGrid(true);

	imageiconscale = IMAGE_ICON_SIZE;
	smoothness	= DEFAULT_SMOOTHNESS;

	InitializeFonts();

	rendermode = RM_FILL;

	numbulbsel = 0;
	initialized = true;

	glwindow.MakeCurrent(MODELLWINDOW);
	LoadToOpenGL_JPG("bulb.jpg", bulbtexture, GL_LINEAR, GL_LINEAR);
	LoadToOpenGL_JPG("texture.jpg", defaulttexture, GL_LINEAR, GL_LINEAR);

	
	glActiveTextureARB		= (PFNGLACTIVETEXTUREARBPROC)		wglGetProcAddress("glActiveTextureARB");
	glMultiTexCoord2fARB	= (PFNGLMULTITEXCOORD2FARBPROC)		wglGetProcAddress("glMultiTexCoord2fARB");

}


void EDITOR::Reset() 
{
	LPARAM point = *((LPARAM *)glwindow.GetInfo(GI_CCENTER, PROJECTIONWINDOW));
	
	vCtr[_X] = vCtr[_Y] = vCtr[_Z] = 0.0f;

	gridSize	= 8;
	mapSize		= 1024;
	VF			= FRONT;

	activeselection = false;

	for (int i=0; i<numobjects; i++)
		object[i].FreeMemory();

	numbulbsel	= 0;
	numbulbs	= 0;
	numobjects	= 0;
	numbrushes	= 0;
	numselected = 0;
	//scalemap	= 1.0f;

	dragvertices	= false;
	rotate			= false;
	SnapToGrid(true);

	tmpbrush.active = false;
	tmpobject.FreeMemory();

	imageiconscale = IMAGE_ICON_SIZE;
	smoothness	= DEFAULT_SMOOTHNESS;

	designstatus = DS_NONE;

	HWND hWnd = *(HWND *)glwindow.GetInfo(GI_HWND);	
	CheckMenuItem(GetMenu(hWnd), ID_EDIT_ROTATE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), ID_SELECT_DRAGVERTICES, MF_UNCHECKED);
}


void EDITOR::DeInitEditor() 
{
	FlushTextures();
	
	for (int i=0; i<numobjects; i++)
		object[i].FreeMemory();

	font.DestroyFont();
	glwindow.DeInit();
}


EDITOR::EDITOR() {
	selectedtexture = NONE;
	initialized = false;
}

void EDITOR::SetTextureWindowScroll(int scroll) {
	texturewindowscroll = scroll;
}

bool EDITOR::GetSmoothness(int &sm) {
	if (numbrushes>0 || !tmpbrush.active) return false;

	sm = tmpobject.GetSmoothness();

	return true;
}

void EDITOR::SetSmoothness(int sm) {
	if (numbrushes>0 || !tmpbrush.active) return;

	tmpobject.SetSmoothness(sm);
	Recalc(tmpobject);
}

void EDITOR::CalculateTextureScroll() {
	float x = IMAGE_ICON_SPACEING;
	float y = IMAGE_ICON_SPACEING;
	float maxy = y;
	float tw;
	float th;

	int		width	= LOWORD(*(LPARAM *)glwindow.GetInfo(GI_CSIZE, TEXTUREWINDOW));

	for (int i=0; i<numtextures; i++) {
		tw = texture[i].width * imageiconscale;
		th = texture[i].height * imageiconscale;

		if (x+tw > width) {
			x = IMAGE_ICON_SPACEING;
			y = maxy;
		}

		texture[i].x = x;
		texture[i].y = y;

		x = x + tw + IMAGE_ICON_SPACEING;

		if (y + th + IMAGE_ICON_SPACEING> maxy)
			maxy = y + th + IMAGE_ICON_SPACEING;

		if (x > width) {
			x = IMAGE_ICON_SPACEING;
			y = maxy;
		}
	}

	SCROLLINFO si;
	si.cbSize = sizeof(SCROLLINFO);
	si.nMin = 0;
	si.nMax = (int)maxy / SCROLL_SIZE;
	si.nPage = 1;
	si.fMask = SIF_PAGE | SIF_RANGE;
	SetScrollInfo(*(HWND *)glwindow.GetInfo(GI_CHWND, TEXTUREWINDOW), SB_VERT, &si, false);
}

void EDITOR::ApplyNewTexture() {
	for (int i=0; i<numbrushes; i++)
		object[brush[i].object].SetTextureImage(texture[selectedtexture]);

	if (tmpbrush.active) {
		tmpobject.SetTextureImage(texture[selectedtexture]);
	}
}

void EDITOR::PickTexture(int x, int y) {
	y += SCROLL_SIZE * texturewindowscroll;

	for (int i=0; i<numtextures; i++) {
		if (texture[i].x<=x && x<=texture[i].x+texture[i].width*imageiconscale &&
			texture[i].y<=y && y<=texture[i].y+texture[i].height*imageiconscale) {
			selectedtexture = i;
			ApplyNewTexture();
			return;
		}
	}
}


void EDITOR::SetShowBulbs(bool flag) {
	showbulbs = flag;
	CheckMenuItem(GetMenu(*(HWND *)glwindow.GetInfo(GI_HWND)), ID_VIEW_BULBS, flag ? MF_CHECKED:MF_UNCHECKED);	
}

void EDITOR::AppendObject(char *filename) {
	if (filename==NULL) return;

	FILE *f = fopen(filename, "rb");

	if (f==NULL) {
		MessageBox(NULL, "Couldn't create file!", "ERROR", MB_ICONEXCLAMATION);
		return;
	}

	_3DW_HEADER	fheader;

	fread(&fheader, sizeof(_3DW_HEADER), 1, f);

	numbulbs += fheader.numlights;
	
	//////////////
	_3DW_OBJECT fobject;

	CVector3	*vbuff3;
	CVector2	*vbuff2;
	int			*tex_idx;
	int			*obj_idx;

	tex_idx = new int[fheader.numobjects];
	obj_idx = new int[fheader.numobjects];

	for (int i=0; i<fheader.numobjects; i++) {
		fread(&fobject, sizeof(_3DW_OBJECT), 1, f);

		vbuff3 = new CVector3[NUM_CONTROLS[fobject.type]];
		fread(vbuff3, sizeof(CVector3), NUM_CONTROLS[fobject.type], f);
		
		object[numobjects].SetType(fobject.type);
		object[numobjects].SetSmoothness(fobject.smoothness);
		object[numobjects].SetControlPoints(vbuff3);
		object[numobjects].tex_shift = fobject.tex_shift;
		object[numobjects].tex_size = fobject.tex_size;	
		object[numobjects].GetPTextureInfo()->transparency = fobject.transparency;
		object[numobjects].SetInitialized(true);
		tex_idx[i] = fobject.texture_index;
		obj_idx[i] = numobjects;
		Recalc(object[numobjects]);
		numobjects++;
	}

	for (i=0; i<fheader.numlights; i++)
		fread(&bulb[i], sizeof(_3DW_LIGHT), 1, f);

	_3DW_TEXTURE	fimage;
	char			fname[MAX_PATH];
	UINT			glID;

	glwindow.MakeCurrent(MODELLWINDOW);

	for (i=0; i<fheader.numtextures; i++) {
		fread(&fimage, sizeof(_3DW_TEXTURE), 1, f);

		fread(fname, sizeof(char), fimage.namesize, f);
		LoadToOpenGL_JPG(fname, glID, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		for (int j=0; j<fheader.numobjects; j++) {
			if (tex_idx[j]==i) {
				object[obj_idx[j]].GetPTextureInfo()->gl_tex_ID[0] = glID;
				object[obj_idx[j]].GetPTextureInfo()->num_textures = 1;
				strcpy(object[obj_idx[j]].GetPTextureInfo()->fname[0], fname);
			}
		}
	}

	fclose(f);
}


void EDITOR::LoadFile(char *filename) {
	if (filename==NULL) return;

	FILE *f = fopen(filename, "rb");

	if (f==NULL) {
		MessageBox(NULL, "Couldn't create file!", "ERROR", MB_ICONEXCLAMATION);
		return;
	}

	Reset();

	_3DW_HEADER	fheader;

	fread(&fheader, sizeof(_3DW_HEADER), 1, f);


	numbulbs = fheader.numlights;

	//////////////
	_3DW_OBJECT fobject;

	CVector3	*vbuff3;
	CVector2	*vbuff2;
	int			*tex_idx;

	tex_idx = new int[fheader.numobjects];

	for (int i=0; i<fheader.numobjects; i++) {
		fread(&fobject, sizeof(_3DW_OBJECT), 1, f);

		vbuff3 = new CVector3[NUM_CONTROLS[fobject.type]];
		fread(vbuff3, sizeof(CVector3), NUM_CONTROLS[fobject.type], f);
		
		object[numobjects].SetType(fobject.type);
		object[numobjects].SetSmoothness(fobject.smoothness);
		object[numobjects].SetControlPoints(vbuff3);
		object[numobjects].tex_shift = fobject.tex_shift;
		object[numobjects].tex_size = fobject.tex_size;
		object[numobjects].GetPTextureInfo()->transparency = fobject.transparency;
		object[numobjects].SetInitialized(true);
		tex_idx[i] = fobject.texture_index;
		Recalc(object[numobjects]);
		numobjects++;
	}

	fread(bulb, sizeof(_3DW_LIGHT), fheader.numlights, f);

	_3DW_TEXTURE	fimage;
	char			fname[MAX_PATH];
	UINT			glID;

	glwindow.MakeCurrent(MODELLWINDOW);

	for (i=0; i<fheader.numtextures; i++) {
		fread(&fimage, sizeof(_3DW_TEXTURE), 1, f);

		fread(fname, sizeof(char), fimage.namesize, f);
		LoadToOpenGL_JPG(fname, glID, GL_LINEAR, GL_LINEAR_MIPMAP_LINEAR);

		for (int j=0; j<fheader.numobjects; j++) {
			if (tex_idx[j]==i) {
				object[j].GetPTextureInfo()->gl_tex_ID[0] = glID;
				object[j].GetPTextureInfo()->num_textures = 1;
				strcpy(object[j].GetPTextureInfo()->fname[0], fname);
			}
		}
	}

	fclose(f);
}


void EDITOR::SaveFile(char *filename) {
	if (filename==NULL) return;
	
	FILE *f = fopen(filename, "wb");

	if (f==NULL) {
		MessageBox(NULL, "Couldn't create file!", "ERROR", MB_ICONEXCLAMATION);
		return;
	}

	// Generating texture name list

	char	texture_names[MAX_TEXTURES][MAX_PATH];
	int		texture_idx[MAX_OBJECTS];
	int		num_tnames = 0;
	
	int		found;

	for (int i=0; i<numobjects; i++) {
		
		found = NONE;

		if (object[i].GetPTextureInfo()->num_textures>0) {

			for (int jj=0; jj<num_tnames && found==NONE; jj++) 
				if (strcmp(texture_names[jj], object[i].GetPTextureInfo()->fname[0])==0) {				
					found = jj;
				}

			if (found==NONE) {
				if (strlen(object[i].GetPTextureInfo()->fname[0])) {
					strcpy(texture_names[jj], object[i].GetPTextureInfo()->fname[0]);
					texture_idx[i] = num_tnames;
					num_tnames++;
				}
			} else {			
				texture_idx[i] = found;
			}
		}
	}


	// Saving world to file

	_3DW_HEADER	fheader;

	fheader.numlights = numbulbs;
	fheader.numobjects = numobjects;
	fheader.numtextures = num_tnames;
	
	fwrite(&fheader, sizeof(_3DW_HEADER), 1, f);

	_3DW_OBJECT	fobject;

	
	for (i=0; i<fheader.numobjects; i++) {
		fobject.smoothness = object[i].GetSmoothness();
		fobject.texture_index = texture_idx[i];
		fobject.type = object[i].GetType();
		fobject.tex_shift = object[i].tex_shift;
		fobject.tex_size = object[i].tex_size;
		fobject.transparency = object[i].GetPTextureInfo()->transparency;

		fwrite(&fobject, sizeof(_3DW_OBJECT), 1, f);
		fwrite(object[i].GetControlPointList(), sizeof(CVector3), NUM_CONTROLS[object[i].GetType()], f);
	}

	fwrite(bulb, sizeof(_3DW_LIGHT), fheader.numlights, f); 



	_3DW_TEXTURE	fimage;
	
	for (i=0; i<num_tnames; i++) {
		fimage.namesize = strlen(texture_names[i])+1;
		fwrite(&fimage, sizeof(_3DW_TEXTURE), 1, f);

		fwrite(texture_names[i], sizeof(char), fimage.namesize, f);
	}

	fflush(f);
	fclose(f);
}


void EDITOR::AddTexture(char filename[MAX_NAME]) {
	if (numtextures>MAX_TEXTURES) {
		PrintLog("Too Many Textures");
		return;
	}

    tImageJPG *img = LoadJPG(filename);


	if (img==NULL) {
		PrintLog("Couldn't Load %s", filename);
		return;
	}

	if (!IsPowOf2(img->sizeX) || !IsPowOf2(img->sizeY)) {
		PrintLog("The image size is not a power of 2... %s ", filename);
	} else {

		glwindow.MakeCurrent(MODELLWINDOW);

		glGenTextures(1, &texture[numtextures].glID_OBJ);
		glBindTexture(GL_TEXTURE_2D, texture[numtextures].glID_OBJ);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, img->sizeX, img->sizeY, GL_RGB, GL_UNSIGNED_BYTE, img->data);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_NEAREST);

		glwindow.MakeCurrent(TEXTUREWINDOW);

		glGenTextures(1, &texture[numtextures].glID);
		glBindTexture(GL_TEXTURE_2D, texture[numtextures].glID);
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, img->sizeX, img->sizeY, GL_RGB, GL_UNSIGNED_BYTE, img->data);

		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR_MIPMAP_NEAREST);	

		strcpy(texture[numtextures].filename, filename);
		texture[numtextures].width  = img->sizeX;
		texture[numtextures].height = img->sizeY;

		numtextures++;
	}

	if (img->data)	{
			free(img->data);
	}

	free(img);
}


void EDITOR::FlushTextures() {
	selectedtexture = NONE;

	glwindow.MakeCurrent(TEXTUREWINDOW);

	for (int i=0; i<numtextures; i++) 
		glDeleteTextures(1, &texture[i].glID);


	glwindow.MakeCurrent(MODELLWINDOW);

	numtextures = 0;
}


void EDITOR::LockTextureRotation(bool flag) {
	locktexrotation = flag;

	CheckMenuItem(GetMenu(*(HWND*)glwindow.GetInfo(GI_HWND)), ID_TEXTURES_LOCKTEXTURE_ROTATIONS, locktexrotation ? MF_CHECKED:MF_UNCHECKED);
}

void EDITOR::LockTextureMove(bool flag) {
	locktexmove = flag;

	CheckMenuItem(GetMenu(*(HWND*)glwindow.GetInfo(GI_HWND)), ID_TEXTURES_LOCKTEXTURE_MOVES, locktexmove ? MF_CHECKED:MF_UNCHECKED);
}

bool EDITOR::Actualize() {
	bool	accepted = false;

	button1D = false;
	button2D = false;

	if (!tmpbrush.active && numbrushes==0) {
		designstatus = DS_NONE;
	}

	switch(designstatus) {
		case DS_NONE:
			if (tmpbrush.active) {
				designstatus = DS_MODIFY_OBJECTS;
				accepted = true;
			}
			break;
		case DS_MODIFY_OBJECTS:
				if (dragvertices) {
					numselected = 0;
					designstatus = DS_SELECT_CONTROLS;
					accepted = true;
				}
				if (rotate && !dragvertices) {
					designstatus = DS_ROTATE_OBJECTS;
					accepted = true;
				}
				if (scaleobjects && !rotate && !dragvertices) {
				designstatus = DS_SCALE_OBJECTS;
				accepted = true;
				}
			break;
		case DS_SCALE_OBJECTS:
			if (!scaleobjects) {
				designstatus = DS_MODIFY_OBJECTS;
			}
			break;
		case DS_SELECT_CONTROLS:
			if (!dragvertices) {
				if (rotate) designstatus = DS_ROTATE_OBJECTS;
				else designstatus = DS_MODIFY_OBJECTS;
			} else if (numselected>0) {
				if (rotate) designstatus = DS_ROTATE_CONTROLS;
				else designstatus = DS_MODIFY_CONTROLS;
			}
			accepted = true; 
			break;
		case DS_MODIFY_CONTROLS:
			if (!dragvertices) {
				accepted = true;
				if (rotate) designstatus = DS_ROTATE_OBJECTS;
				else designstatus = DS_MODIFY_OBJECTS;
			} else {				
				if (rotate) {
					accepted = true;
					designstatus = DS_ROTATE_CONTROLS;
				}
			}
			break;
		case DS_ROTATE_OBJECTS:
			if (dragvertices) { 
				numselected = 0;
				designstatus = DS_SELECT_CONTROLS;
				accepted = true;
			}
			if (!rotate && !dragvertices) {
				designstatus = DS_MODIFY_OBJECTS;
				accepted = true;
			}
			break;
		case DS_ROTATE_CONTROLS:
			if (!rotate && dragvertices) {
				designstatus = DS_MODIFY_CONTROLS;
				accepted = true;
			}
			if (!dragvertices) {
				if (rotate) designstatus = DS_ROTATE_OBJECTS;
				else designstatus = DS_MODIFY_OBJECTS;
				accepted = true;
			}
			break;
	}


	return accepted;
}

void EDITOR::DeleteBulb(int idx) {
	for (int i=idx; i<numbulbs-1; i++) {
		bulb[i] = bulb[i+1];
	}
	numbulbs--;
}

void EDITOR::DeleteSelectedObject() {
	if (!tmpbrush.active && numbrushes==0 && numbulbsel==0) return;

	if (tmpbrush.active) {
		tmpobject.FreeMemory();
		tmpobject.Reset(true);		
		tmpbrush.active=false;
	}

	int idx[MAX_BRUSHES];

	////////////////////////////////////////////
	for (int i=0; i<numbrushes; i++) 
		idx[i] = brush[i].object;

	SORT s;
	s.StartSort(idx,numbrushes);
	

	for (i=0; i<numbrushes; i++)
		DeleteObject(idx[i], true);

	numbrushes = 0;

	/////////////////////////////////////////
	
	s.StartSort(bulb_sel, numbulbsel);

	for (i=0; i<numbulbsel; i++) {
		DeleteBulb(bulb_sel[i]);
	}

	numbulbsel = 0;

	rotate = false;
	dragvertices = false;
	scaleobjects = false;
	///////////////////////////////////////

	Actualize();
}

void EDITOR::DuplicateSelectedObject() {
	if (!tmpbrush.active && numbrushes==0) return;

	if (tmpbrush.active) {
		object[numobjects] = tmpobject;
		object[numobjects].Reset(false);
		object[numobjects].SetData(tmpobject);
		numobjects++;
	}

	for (int i=0; i<numbrushes; i++) {
		object[numobjects] = object[brush[i].object];
		object[numobjects].Reset(false);
		object[numobjects].SetData(object[brush[i].object]);
		numobjects++;
	}
}


void EDITOR::SetShowLightmaps(bool flag) {
	showlightmaps = flag;
	CheckMenuItem(GetMenu(*(HWND*)glwindow.GetInfo(GI_HWND)), ID_VIEW_SHOWLIGHTMAPS, flag ? MF_CHECKED:MF_UNCHECKED);
}


void EDITOR::BackStatus() {
	HWND hWnd = *(HWND *)glwindow.GetInfo(GI_HWND);

	button1D = false;
	button2D = false;

	switch(designstatus) {
		case DS_NONE:
			break;
		case DS_MODIFY_OBJECTS:
			if (tmpbrush.active) {
				object[numobjects] = tmpobject;
				numobjects++;
			}

			numbulbsel = 0;

			tmpobject.Reset(true);
			numbrushes = 0;
			designstatus = DS_NONE;
			break;
		case DS_SELECT_CONTROLS:
			if (rotate)	designstatus = DS_ROTATE_OBJECTS;
			else designstatus = DS_MODIFY_OBJECTS;

			dragvertices = false;
			CheckMenuItem(GetMenu(hWnd), ID_SELECT_DRAGVERTICES, MF_UNCHECKED);
			break;
		case DS_MODIFY_CONTROLS:
			if (numselected>0) {
				numselected = 0;
				designstatus = DS_SELECT_CONTROLS;
			} else {
				dragvertices = false;
				CheckMenuItem(GetMenu(hWnd), ID_SELECT_DRAGVERTICES, MF_UNCHECKED);
				designstatus = DS_MODIFY_OBJECTS;
			}
			break;
		case DS_SCALE_OBJECTS:
			designstatus = DS_MODIFY_OBJECTS;
			break;
		case DS_ROTATE_OBJECTS:
			designstatus = DS_MODIFY_OBJECTS;
			
			rotate = false;
			CheckMenuItem(GetMenu(hWnd), ID_EDIT_ROTATE, MF_UNCHECKED);
			break;
		case DS_ROTATE_CONTROLS:
			rotate = false;
			designstatus = DS_MODIFY_CONTROLS;			
			CheckMenuItem(GetMenu(hWnd), ID_EDIT_ROTATE, MF_UNCHECKED);
			break;
	}

}


void EDITOR::SnapToGrid(bool snap) {
	snaptogrid = snap;

	HWND hWnd = *(HWND*)glwindow.GetInfo(GI_HWND);
	CheckMenuItem(GetMenu(hWnd), ID_EDIT_SNAPTOGRID, snaptogrid ? MF_CHECKED:MF_UNCHECKED);
}


bool EDITOR::EnableScaleObjects(bool flag) {
	if (numbrushes==0 && !tmpbrush.active) return false;

	scaleobjects = flag;

	if (scaleobjects) gpoint = GetHub();

	return Actualize();
}


bool EDITOR::Rotation(bool flag) {
	if (numbrushes==0 && !tmpbrush.active) return false;

	
	if (designstatus!=DS_NONE) {
		rotate = flag;
		if (rotate) gpoint = GetHub();

		if (rotate) scaleobjects = true;

		return Actualize();
	} else return false;
}


bool EDITOR::DragVertices(bool flag) {
	if (numbrushes==0 && !tmpbrush.active) return false;

	dragvertices = flag;
	
	if (dragvertices) scaleobjects = true;

	return Actualize();
}

void EDITOR::Transform2DToPolar(float *x, float *y, float factor) {
	LPARAM size = *((LPARAM *)glwindow.GetInfo(GI_CSIZE, PROJECTIONWINDOW));

	int xx = LOWORD(size);
	int yy = HIWORD(size);

	*x *= scalemap;
	*y *= scalemap;

	*x = xx/2 + *x;
	*y = yy/2 - *y;

	*x += factor*vCtr[GA(VF,0)];
	*y += factor*vCtr[GA(VF,1)];
}

void EDITOR::TransformWCTo2DPolar(float *x, float *y, float factor) {
	LPARAM size = *((LPARAM *)glwindow.GetInfo(GI_CSIZE, PROJECTIONWINDOW));

	int xx = LOWORD(size);
	int yy = HIWORD(size);

	*x = *x - xx/2;
	*y = yy/2 - *y;

	*x -= factor*vCtr[GA(VF,0)];
	*y += factor*vCtr[GA(VF,1)];
}

void EDITOR::TransformWCTo1DPolar(float *y, float factor) {
	LPARAM size = *((LPARAM *)glwindow.GetInfo(GI_CSIZE, PROJECTIONWINDOW));

	int yy = HIWORD(size);
	*y = yy/2 - *y;
	*y += factor*vCtr[VF];
}


void EDITOR::Transform1DToPolar(float *z, float factor) {
	LPARAM size = *((LPARAM *)glwindow.GetInfo(GI_CSIZE, ONEDWINDOW));

	int yy = HIWORD(size);

	*z *= scalemap;

	*z = yy/2 - *z;

	*z += factor*vCtr[VF];
}

void EDITOR::CreateObject(int type) {
	if (!tmpbrush.active) return;

	switch(type) {
		case ST_BCAP:
			if (!tmpbrush.active || tmpobject.GetType()!=ST_BEVEL) return;

			CreateObject(object[numobjects], ST_BCAP, false);
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;
			

			CreateObject(object[numobjects], ST_BCAP, true);
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;

			object[numobjects] = tmpobject;
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;

			tmpobject.Reset();
			tmpbrush.active=false;
			break;
		case ST_ECAP:
			if (!tmpbrush.active || tmpobject.GetType()!=ST_ENDCAP) return;

			CreateObject(object[numobjects], ST_ECAP, false);
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;

			CreateObject(object[numobjects], ST_ECAP, true);
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;

			object[numobjects] = tmpobject;
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;
			
			tmpobject.Reset();
			tmpbrush.active=false;
			break;
		case ST_IBCAP:
			if (!tmpbrush.active || tmpobject.GetType()!=ST_BEVEL) return;

			CreateObject(object[numobjects], ST_IBCAP, false);
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;
			

			CreateObject(object[numobjects], ST_IBCAP, true);
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;

			object[numobjects] = tmpobject;
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;

			tmpobject.Reset();
			tmpbrush.active=false;
			break;
		case ST_IECAP:
			if (!tmpbrush.active || tmpobject.GetType()!=ST_ENDCAP) return;

			CreateObject(object[numobjects], ST_IECAP, false);
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;

			CreateObject(object[numobjects], ST_IECAP, true);
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;

			object[numobjects] = tmpobject;
			brush[numbrushes] = object[numobjects].GetBBoxO();
			brush[numbrushes].object = numobjects;
			numbrushes++;
			numobjects++;
			
			tmpobject.Reset();
			tmpbrush.active=false;
			break;
		default:			
			CreateObject(tmpobject, type);
			tmpbrush = tmpobject.GetBBoxO();
	}

	dragvertices = false;
	rotate = false;
	scaleobjects = false;
	Actualize();
	
	HWND hWnd = *(HWND *)glwindow.GetInfo(GI_HWND);	
	CheckMenuItem(GetMenu(hWnd), ID_EDIT_ROTATE, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), ID_SELECT_DRAGVERTICES, MF_UNCHECKED);
	CheckMenuItem(GetMenu(hWnd), ID_EDIT_SCALEOBJECTS, MF_UNCHECKED);
}

void EDITOR::CreateObject(OBJECT &obj, int type, bool lowhigh) {
	
	if (numobjects>=MAX_OBJECTS || !tmpbrush.active) return;

	CVector3	start;
	CVector3	end;

	start	= CVector3(tmpbrush.start[_X], tmpbrush.start[_Y], tmpbrush.start[_Z]);
	end		= CVector3(tmpbrush.end[_X], tmpbrush.end[_Y], tmpbrush.end[_Z]);
	
	BEVEL		bevel;
	BOX			box;
	IBCAP		ibcap;
	IECAP		iecap;
	ENDCAP		endcap;
	NCILINDER	ncilinder;
	NCONE		ncone;
	NPLANE		nplane;
	SPHERE		sphere;
	ECAP		ecap;
	BCAP		bcap;
	

	BRUSH	tmp;
	bool	valid = false;

	switch(type) {
		case ST_BEVEL:			
			bevel.Init(start, end, smoothness);
			bevel.GenerateShape();
			obj.SetData(bevel);
			valid = true;
			break;
		case ST_BOX:
			box.Init(start, end);
			box.SetSmoothness(smoothness);
			box.GenerateShape();
			obj.SetData(box);
			valid = true;
			break;
		case ST_ENDCAP:
			endcap.Init(start, end, smoothness);
			endcap.GenerateShape();
			obj.SetData(endcap);
			valid = true;
			break;
		case ST_NCILINDER:
			ncilinder.Init(start, end, smoothness);
			ncilinder.GenerateShape();
			obj.SetData(ncilinder);
			valid = true;
			break;
		case ST_NCONE:
			ncone.Init(start, end, smoothness);
			ncone.GenerateShape();
			obj.SetData(ncone);
			valid = true;
			break;
		case ST_NPLANE:
			nplane.Init(start, end, smoothness, VF);
			nplane.GenerateShape();
			obj.SetData(nplane);
			valid = true;
			break;
		case ST_SPHERE:
			sphere.Init(start, end, smoothness);
			sphere.GenerateShape();
			obj.SetData(sphere);
			valid = true;
			break;
		case ST_BCAP:
			if (tmpobject.GetType()!=ST_BEVEL) return;
			bcap.Init(tmpobject, lowhigh);
			bcap.GenerateShape();
			obj.SetData(bcap);
			valid = true;
			break;
		case ST_ECAP:
			if (tmpobject.GetType()!=ST_ENDCAP) return;			
			ecap.Init(tmpobject, lowhigh);
			ecap.GenerateShape();
			obj.SetData(ecap);
			valid = true;
			break;
		case ST_IBCAP:
			if (tmpobject.GetType()!=ST_BEVEL) return;
			ibcap.Init(tmpobject, lowhigh);
			ibcap.GenerateShape();
			obj.SetData(ibcap);
			valid = true;
			break;
		case ST_IECAP:
			if (tmpobject.GetType()!=ST_ENDCAP) return;			
			iecap.Init(tmpobject, lowhigh);
			iecap.GenerateShape();
			obj.SetData(iecap);
			valid = true;
			break;
	}

	obj.SetOrientation(tmpbrush.GetOrientation());

	if (valid) {
		tmp = obj.GetBBoxN();
		obj.SetCenter(CVector3((tmp.start[_X]+tmp.end[_X])/2,
							   (tmp.start[_Y]+tmp.end[_Y])/2,
							   (tmp.start[_Z]+tmp.end[_Z])/2));
	}

	obj.FitTexture();
}





void EDITOR::DrawAxes(bool is2D) 
{
	int		max		= mapSize;
	int		step	= gridSize;
	LPARAM	size	= *((LPARAM *)glwindow.GetInfo(GI_CSIZE, is2D ? PROJECTIONWINDOW:ONEDWINDOW));

	int		width	= LOWORD(size);
	int		height	= HIWORD(size);

	float	cx = vCtr[GA(VF,0)];
	float	cy = is2D ? vCtr[GA(VF,1)]:vCtr[VF];

	glColor3f(0,0,0);

	int majorgridsize = MAJOR_GRID_SIZE * scalemap;

	int X = ((width/2)/majorgridsize + 2)*majorgridsize;
	int Y = ((height/2)/majorgridsize + 2)*majorgridsize;

	float x, y;

	step *= scalemap;

	if (step<MIN_GRID_SIZE) step = MAJOR_GRID_SIZE * scalemap;

	if (is2D) {
		// vertical grid lines
		for (int i = -X; i<=X; i+=step) {
			if (i%(int)(majorgridsize)) {
				glColor3f(0.8f, 0.8f, 0.8f);
			} else {
				glColor3f(0, 0, 0);
			}

			glBegin(GL_LINES);
				x = width/2 + i + (int)cx % majorgridsize;
				y = 0;
				glVertex2f(x,y);
				x = width/2 + i + (int)cx % majorgridsize;
				y = height;
				glVertex2f(x,y);
			glEnd();
		}
	}

	// horizontal grid lines
	for (int i = -Y; i<=Y; i+=step) {
		if (i%(int)(majorgridsize)) {
			glColor3f(0.8f, 0.8f, 0.8f);
		} else {
			glColor3f(0, 0, 0);
		}

		glBegin(GL_LINES);
			x = 0;
			y = height/2 + i + (int)cy % majorgridsize;
			glVertex2f(x,y);
			x = width;
			y = height/2 + i + (int)cy % majorgridsize;
			glVertex2f(x,y);
		glEnd();
	}

	glColor3f(0, 0, 0);

	if (is2D) {
		for (i = -X; i<=X; i+=majorgridsize) {
			x = width/2 + i + (int)cx % majorgridsize;
			y = 0;
			font.glPrint(x+3, y, "%2.0f", (i - ((int)(cx)/majorgridsize)*majorgridsize) / scalemap);
		}
	}

	for (i = -Y; i<=Y; i+=majorgridsize) {
		x = GL_FONT_HEIGHT/3;
		y = height/2 + i + (int)cy % majorgridsize;
		font.glPrint(x, y - GL_FONT_HEIGHT-3, "%2.0f",-(i - ((int)(cy)/majorgridsize)*majorgridsize) / scalemap);
	}	
}


void EDITOR::DrawObject2D(OBJECT obj) {
	// rendering object projections
	CVector3	*vbuff3;
	Triangle	*tbuff;
	int			numtriangles;
	float		v[3];
	float		x,y;

	numtriangles = obj.GetNumTriangles();
	vbuff3 = obj.GetVertexList();
	tbuff = obj.GetTriangleList();

	for (int jj=0; jj<numtriangles; jj++) {
		glBegin(GL_LINES);
			v[_X] = vbuff3[tbuff[jj].p1].x;
			v[_Y] = vbuff3[tbuff[jj].p1].y;
			v[_Z] = vbuff3[tbuff[jj].p1].z;
			x = v[GA(VF,0)];
			y = v[GA(VF,1)];
			Transform2DToPolar(&x,&y,1);
			glVertex2f(x,y);

			v[_X] = vbuff3[tbuff[jj].p2].x;
			v[_Y] = vbuff3[tbuff[jj].p2].y;
			v[_Z] = vbuff3[tbuff[jj].p2].z;
			x = v[GA(VF,0)];
			y = v[GA(VF,1)];
			Transform2DToPolar(&x,&y,1);
			glVertex2f(x,y);
			glVertex2f(x,y);

			v[_X] = vbuff3[tbuff[jj].p3].x;
			v[_Y] = vbuff3[tbuff[jj].p3].y;
			v[_Z] = vbuff3[tbuff[jj].p3].z;
			x = v[GA(VF,0)];
			y = v[GA(VF,1)];
			Transform2DToPolar(&x,&y,1);
			glVertex2f(x,y);
			glVertex2f(x,y);

			v[_X] = vbuff3[tbuff[jj].p1].x;
			v[_Y] = vbuff3[tbuff[jj].p1].y;
			v[_Z] = vbuff3[tbuff[jj].p1].z;
			x = v[GA(VF,0)];
			y = v[GA(VF,1)];
			Transform2DToPolar(&x,&y,1);
			glVertex2f(x,y);
		glEnd();
	}
}


void EDITOR::DrawObject3D(OBJECT obj, bool texture) {
	if (!obj.IsInitialized()) return;

	CVector3	*vbuff3;
	CVector2	*vbuff2;
	Triangle	*tbuff;
	int			numtriangles;
	float		transparency;
	bool		bb;

	tbuff = obj.GetTriangleList();
	vbuff3 = obj.GetVertexList();
	vbuff2 = obj.GetTexCoordList();
	numtriangles = obj.GetNumTriangles();


	glActiveTextureARB(GL_TEXTURE0_ARB);
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,  obj.GetType()==ST_BULB ? bulbtexture:obj.GetGLTexID());
	
	glMatrixMode(GL_TEXTURE);
		glLoadIdentity();
		glRotatef(180, 1,0,0);
	glMatrixMode(GL_MODELVIEW);

	transparency = obj.GetPTextureInfo()->transparency;
	bb = obj.GetPTextureInfo()->billboarding;
	
	if (texture) glEnable(GL_TEXTURE_2D);
	else glDisable(GL_TEXTURE_2D);

	switch (rendermode) {
		case RM_POINT: 
			glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
			glDisable(GL_TEXTURE_2D);
			break;
		case RM_LINE: 
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glDisable(GL_TEXTURE_2D);
			break;
		case RM_FILL: 
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			break;
	}

	if (transparency>0.0f) {
		glColor4f(1, 1, 1, transparency);		
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		
		if (bb) glBlendFunc(GL_ZERO, GL_SRC_COLOR);
		else glBlendFunc(GL_ONE, GL_ONE);

		glDepthFunc(GL_LEQUAL);
	}

	glRenderMode(GL_FRONT);

	CVector2 tex = obj.GetTextureSize();
	CVector2 sht = obj.GetTextureShift();
	
	for (int j=0; j<numtriangles; j++) {
		if (showlightmaps) {
			glActiveTextureARB(GL_TEXTURE1_ARB);
			glEnable(GL_TEXTURE_2D);
			glBindTexture(GL_TEXTURE_2D,  tbuff[j].l_ID);
		}	

		glBegin(GL_TRIANGLES);
			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, sht.x + tex.x * vbuff2[tbuff[j].p1].x, sht.y + tex.y * vbuff2[tbuff[j].p1].y);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, tbuff[j].lightmap[0].x, tbuff[j].lightmap[0].y);
			glVertex3f(vbuff3[tbuff[j].p1].x,vbuff3[tbuff[j].p1].y,vbuff3[tbuff[j].p1].z);

			glMultiTexCoord2fARB(GL_TEXTURE0_ARB, sht.x + tex.x * vbuff2[tbuff[j].p2].x, sht.y + tex.y * vbuff2[tbuff[j].p2].y);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, tbuff[j].lightmap[1].x, tbuff[j].lightmap[1].y);
			glVertex3f(vbuff3[tbuff[j].p2].x,vbuff3[tbuff[j].p2].y,vbuff3[tbuff[j].p2].z);


			glMultiTexCoord2fARB(GL_TEXTURE0_ARB,sht.x + tex.x * vbuff2[tbuff[j].p3].x, sht.y + tex.y * vbuff2[tbuff[j].p3].y);
			glMultiTexCoord2fARB(GL_TEXTURE1_ARB, tbuff[j].lightmap[2].x, tbuff[j].lightmap[2].y);
			glVertex3f(vbuff3[tbuff[j].p3].x,vbuff3[tbuff[j].p3].y,vbuff3[tbuff[j].p3].z);
		glEnd();
	}

	if (transparency>0.0f) glDisable(GL_BLEND);
}


void EDITOR::DrawEye(bool is2D) {
	CVector3 pos = camera.GetPos() * (COORD_UNIT);
	float v[3];
	float px, py;
	float x, y;


	if (is2D) {
		v[_X] = pos.x;
		v[_Y] = pos.y;
		v[_Z] = pos.z;

		px = v[GA(VF,0)];
		py = v[GA(VF,1)];

		glDisable(GL_TEXTURE_2D);
		glColor3f(0.0f, 0.0f, 1.0f);

		glBegin(GL_LINES);
			x = px; y = py;
			Transform2DToPolar(&x,&y,1);
			glVertex2f(x, y + BULB_ICON_SIZE/2);
			glVertex2f(x + BULB_ICON_SIZE/2, y);

			glVertex2f(x + BULB_ICON_SIZE/2, y);
			glVertex2f(x, y - BULB_ICON_SIZE/2);

			glVertex2f(x, y - BULB_ICON_SIZE/2);
			glVertex2f(x - BULB_ICON_SIZE/2, y);

			glVertex2f(x - BULB_ICON_SIZE/2, y);
			glVertex2f(x, y + BULB_ICON_SIZE/2);
		glEnd();
	} else {
		v[_X] = pos.x;
		v[_Y] = pos.y;
		v[_Z] = pos.z;

		px = v[VF];

		glDisable(GL_TEXTURE_2D);
		glColor3f(0.0f, 0.0f, 1.0f);

		glBegin(GL_LINES);
			x = LOWORD(*(LPARAM *)glwindow.GetInfo(GI_CSIZE, ONEDWINDOW))/2.0f;
			y = px;
			Transform1DToPolar(&y,1);
			glVertex2f(x, y + BULB_ICON_SIZE/2);
			glVertex2f(x + BULB_ICON_SIZE/2, y);

			glVertex2f(x + BULB_ICON_SIZE/2, y);
			glVertex2f(x, y - BULB_ICON_SIZE/2);

			glVertex2f(x, y - BULB_ICON_SIZE/2);
			glVertex2f(x - BULB_ICON_SIZE/2, y);

			glVertex2f(x - BULB_ICON_SIZE/2, y);
			glVertex2f(x, y + BULB_ICON_SIZE/2);
		glEnd();
	}
}


void EDITOR::DrawProjection() 
{
	LPARAM	size	= *((LPARAM *)glwindow.GetInfo(GI_CSIZE, PROJECTIONWINDOW));
	int		width	= LOWORD(size);
	int		height	= HIWORD(size);

	glwindow.MakeCurrent(PROJECTIONWINDOW);
	
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	OrthoMode(0,0,width, height);
	
	DrawEye(true);
	
	// rendering the bounding boxes of the objects that are actually edited
	DrawSelectionMark(tmpobject, tmpbrush, true);

	for (int i=0; i<numbrushes; i++)
		DrawSelectionMark(object[brush[i].object], brush[i], true);


	glColor3f(1,0,0);
	DrawObject2D(tmpobject);

	for (i=0; i<numobjects; i++) {

		glColor3f(0,0,0);

		for (int j=0; j<numbrushes; j++)
			if (brush[j].object==i) {
				glColor3f(1,0,0);
				break;
			}
			
		DrawObject2D(object[i]);
	}
	
	for (i=0; i<numbulbs; i++) {
		glColor3f(0,1,0);
		for (int k=0; k<numbulbsel; k++)
			if (bulb_sel[k] == i) { 
				glColor3f(1,0,0);
			}
		DrawBulb2D(bulb[i]);
	}

	// rendering axes
	DrawAxes(true);	

	glColor3f(0,0,1);
	font.glPrint(20,20, "%c%c", 'X'+GA(VF,0), 'X'+GA(VF,1));

	// draw the selection mark
	if (activeselection) DrawSelection();


	PerspectiveMode();
	glwindow.PaintFrame();
}


void EDITOR::DrawOneD() 
{
	glwindow.MakeCurrent(ONEDWINDOW);
	
	glClearColor(1,1,1,0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	LPARAM	size	= *((LPARAM *)glwindow.GetInfo(GI_CSIZE, ONEDWINDOW));

	int		width	= LOWORD(size);
	int		height	= HIWORD(size);
	
	OrthoMode(0,0,width, height);

	glColor3f(0,0,1);
	font.glPrint(10,10, "%c", 'X'+VF);

	DrawEye(false);
	
	DrawSelectionMark(tmpobject, tmpbrush, false);

	for (int i=0; i<numbrushes; i++)
		DrawSelectionMark(object[brush[i].object], brush[i], false);

	for (i=0; i<numbulbsel;i++)	{
		glColor3f(0,0,1);
		DrawBulb1D(bulb[bulb_sel[i]]);
	}

	DrawAxes(false);
	
	PerspectiveMode();
	glwindow.PaintFrame();
}


void EDITOR::DrawBulb(BULB b) {

	SPHERE box;
	box.Init(b.pos, b.pos + CVector3(BULB_ICON_SIZE,BULB_ICON_SIZE,BULB_ICON_SIZE),3);
	box.GenerateShape();
	
	OBJECT obj;
	obj.SetData(box);
	obj.SetType(ST_BULB);

	glColor3f(1,1,1);
	DrawObject3D(obj);

	obj.FreeMemory();
}

void EDITOR::DrawBulb1D(BULB b) {
	int	width = LOWORD(*((LPARAM *)glwindow.GetInfo(GI_CSIZE,ONEDWINDOW)));
	float y;
	float v[3];
	float sy, ey;

	v[_X] = b.pos.x;
	v[_Y] = b.pos.y;
	v[_Z] = b.pos.z;

	y = v[VF];
	
	sy = y + BULB_ICON_SIZE / 2.0f;
	ey = y - BULB_ICON_SIZE / 2.0f;

	Transform1DToPolar(&ey,1);
	Transform1DToPolar(&sy,1);

	glBegin(GL_LINES);
		glVertex2f(0, sy);
		glVertex2f(width, sy);

		glVertex2f(0, ey);
		glVertex2f(width, ey);
	glEnd();
}


void EDITOR::DrawBulb2D(BULB b) {
	float x,y;
	float v[3];
	float sx,sy,ex,ey;

	v[_X] = b.pos.x;
	v[_Y] = b.pos.y;
	v[_Z] = b.pos.z;

	x = v[GA(VF,0)];
	y = v[GA(VF,1)];
	
	
	sx = x - BULB_ICON_SIZE/2;
	sy = y - BULB_ICON_SIZE/2;
	ex = x + BULB_ICON_SIZE/2;
	ey = y + BULB_ICON_SIZE/2;
	Transform2DToPolar(&sx,&sy,1);
	Transform2DToPolar(&ex,&ey,1);

	glBegin(GL_LINES);
		glVertex2f(sx, sy);		
		glVertex2f(sx, ey);

		glVertex2f(sx, ey);		
		glVertex2f(ex, ey);

		glVertex2f(ex, ey);
		glVertex2f(ex, sy);

		glVertex2f(ex, sy);
		glVertex2f(sx, sy);
	glEnd();

	x -= BULB_ICON_SIZE/3;
	Transform2DToPolar(&x,&y,1);
	font.glPrint(x,y,"Light");
}


void EDITOR::DrawModell() 
{
	glwindow.MakeCurrent(MODELLWINDOW);

	glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	camera.Apply();
	
	glScalef(SCALE_COORDS, SCALE_COORDS, SCALE_COORDS);

	glEnable(GL_DEPTH_TEST);
		
	int i;
		
	for (i=0; i<numobjects; i++) {
		if (object[i].GetPTextureInfo()->transparency==0) {
			glColor3f(1,1,1);
		
			DrawObject3D(object[i]);

			for (int k=0; k<numbrushes; k++)
				if (brush[k].object==i) {
					DrawSelectionMark3D(object[i], object[i].GetBBoxN());
					break;
				}
		}
	}

	
	if (tmpobject.GetPTextureInfo()->transparency==0) {
		glColor3f(1,1,1);
		DrawObject3D(tmpobject);
		if (tmpbrush.active) DrawSelectionMark3D(tmpobject, tmpobject.GetBBoxN());
	}

	if (showbulbs)
	for (i=0; i<numbulbs; i++) DrawBulb(bulb[i]);

	
	////////////////////////////////////////////////////

	for (i=0; i<numobjects; i++) {
		if (object[i].GetPTextureInfo()->transparency>0) {

			glColor3f(1,1,1);
		
			glEnable(GL_CULL_FACE);
			glCullFace(GL_FRONT);
			DrawObject3D(object[i]);
			glCullFace(GL_BACK);
			DrawObject3D(object[i]);
			glDisable(GL_CULL_FACE);

			for (int k=0; k<numbrushes; k++)
				if (brush[k].object==i) {
					DrawSelectionMark3D(object[i], object[i].GetBBoxN());
					break;
				}
		}
	}

	if (tmpobject.GetPTextureInfo()->transparency>0) {

		glColor3f(1,1,1);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		DrawObject3D(tmpobject);
		glCullFace(GL_BACK);
		DrawObject3D(tmpobject);
		glDisable(GL_CULL_FACE);
	
		if (tmpbrush.active) DrawSelectionMark3D(tmpobject, tmpobject.GetBBoxN());
	}
	

	
	
	glwindow.PaintFrame();
}

void EDITOR::GenerateLightMaps() {
	int numtriangles;

	glwindow.MakeCurrent(MODELLWINDOW);

	for (int i=0; i<numobjects; i++) {
		numtriangles = object[i].GetNumTriangles();

		for (int j=0; j<numtriangles; j++) 
			GenerateLightMap(i,j);
	}
}


void EDITOR::GenerateLightMap(int IDX_o, int IDX_t) {
	CVector3	v[3];
	CVector2	lightmap[3];
	CVector3	normal;
	byte		data[LW*LH*3];
	int			flag;
	
	CVector3	*vbuff3;
	TRIANGLE	*tbuff;

	vbuff3 = object[IDX_o].GetVertexList();
	tbuff  = object[IDX_o].GetTriangleList();

	v[0] = vbuff3[tbuff[IDX_t].p1];
	v[1] = vbuff3[tbuff[IDX_t].p2];
	v[2] = vbuff3[tbuff[IDX_t].p3];

	normal = Normal(v);

	//projecting on the plane with the biggest shadow of the poly
	if (fabs(normal.x)>=fabs(normal.y) && fabs(normal.x)>=fabs(normal.z))
	{
		flag=1;
		lightmap[0].x = v[0].y;
		lightmap[0].y = v[0].z;
		lightmap[1].x = v[1].y;
		lightmap[1].y = v[1].z;
		lightmap[2].x = v[2].y;
		lightmap[2].y = v[2].z;	
	} else if (fabs(normal.y)>=fabs(normal.x) && fabs(normal.y)>=fabs(normal.z))
	{
		flag=2;
		lightmap[0].x = v[0].x;
		lightmap[0].y = v[0].z;
		lightmap[1].x = v[1].x;
		lightmap[1].y = v[1].z;
		lightmap[2].x = v[2].x;
		lightmap[2].y = v[2].z;
	} else {
		flag=3;
		lightmap[0].x = v[0].x;
		lightmap[0].y = v[0].y;
		lightmap[1].x = v[1].x;
		lightmap[1].y = v[1].y;
		lightmap[2].x = v[2].x;
		lightmap[2].y = v[2].y;
	}

	float
		Min_U = lightmap[0].x,
		Min_V = lightmap[0].y,
		Max_U = lightmap[0].x,
		Max_V = lightmap[0].y;

	for (int i=0; i<3; i++)
	{
		if (lightmap[i].x < Min_U )
           Min_U = lightmap[i].x;
        if (lightmap[i].y < Min_V )
           Min_V = lightmap[i].y;
        if (lightmap[i].x > Max_U )
           Max_U = lightmap[i].x;
	    if (lightmap[i].y > Max_V )
           Max_V = lightmap[i].y;
	}

	
	float
		Delta_U = Max_U - Min_U,
		Delta_V = Max_V - Min_V;


	// clamp the UV coordonates into [0..1]
	for (i=0; i<3; i++)
	{		
		lightmap[i].x -= Min_U;
		lightmap[i].y -= Min_V;
		lightmap[i].x /= Delta_U;
		lightmap[i].y /= Delta_V;
	}
	

	CVector3 UVVector, Vect1, Vect2, edge1, edge2;
	float X,Y,Z;

	float dst = - (v[0].x * normal.x + v[0].y * normal.y + v[0].z * normal.z);

	// projecting back to the original plane given by GL_mp
	switch (flag)
        {
            case 1: //YZ Plane
				X = - ( normal.y * Min_U + normal.z * Min_V + dst) / normal.x;
                UVVector = CVector3(X, Min_U, Min_V);

				X = - ( normal.y * Max_U + normal.z * Min_V + dst) / normal.x;
                Vect1 = CVector3(X, Max_U, Min_V);

				X = - ( normal.y * Min_U + normal.z * Max_V + dst) / normal.x;
                Vect2 = CVector3(X, Min_U, Max_V);
				break;

            case 2: //XZ Plane
				Y = - ( normal.x * Min_U + normal.z * Min_V + dst) / normal.y;
				UVVector = CVector3(Min_U, Y, Min_V);

				Y = - ( normal.x * Max_U + normal.z * Min_V + dst) / normal.y;
                Vect1 = CVector3(Max_U, Y, Min_V);

				Y = - ( normal.x * Min_U + normal.z * Max_V + dst) / normal.y;
                Vect2 = CVector3(Min_U, Y, Max_V);
				break;

            case 3: //XY Plane
				Z = - ( normal.x * Min_U + normal.y * Min_V + dst) / normal.z;
                UVVector = CVector3(Min_U, Min_V, Z);

				Z = - ( normal.x * Max_U + normal.y * Min_V + dst) / normal.z;
                Vect1 = CVector3(Max_U, Min_V, Z);

				Z = - ( normal.x * Min_U + normal.y * Max_V + dst) / normal.z;
                Vect2 = CVector3(Min_U, Max_V, Z);
				break;
    }


    edge1 = Vect1 - UVVector;
	edge2 = Vect2 - UVVector;
	
  
	CVector3 newedge1, newedge2, tmp;

	
	float distance;
	float intensity;
	float r,g,b;
	float uf, vf;

	// calculating light factors in each region of the
	// LW x LH sized map
	for (i=0; i<LW; i++)
	for (int j=0; j<LH; j++)
	{
		
		    uf = (i+1)/((float)(LW));
			vf = (j+1)/((float)(LH));

			newedge1 = edge1 * uf;
			newedge2 = edge2 * vf;
			
			tmp = UVVector + newedge1 + newedge2;

			r = 0;
			g = 0;
			b = 0;

			for (int k=0; k<numbulbs; k++)
			{
				distance = Magnitude(tmp-bulb[k].pos);

				if (distance < bulb[k].range)
				{
					intensity =(float)(1.0 - distance/(bulb[k].range));

					r +=bulb[k].r * intensity * bulb[k].brightness;
					g +=bulb[k].g * intensity * bulb[k].brightness;
					b +=bulb[k].b * intensity * bulb[k].brightness; 
				}
			}


			if (r>255.0f) r=255.0f;
			if (g>255.0f) g=255.0f;
			if (b>255.0f) b=255.0f;

			data[(i + j*LH)*3]   =(int) r;
			data[(i + j*LH)*3+1] =(int) g;
			data[(i + j*LH)*3+2] =(int) b;
	}


	if (tbuff[IDX_t].l_ID >0 ) glDeleteTextures(1, &tbuff[IDX_t].l_ID);
	AddToOpenGL(data, tbuff[IDX_t].l_ID);
	memcpy(tbuff[IDX_t].lightmap, lightmap, 3*sizeof(CVector2));
}



void EDITOR::DrawTexture() 
{	
	glwindow.MakeCurrent(TEXTUREWINDOW);

	glClearColor(0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();	
	
	
	
	LPARAM	size	= *((LPARAM *)glwindow.GetInfo(GI_CSIZE, TEXTUREWINDOW));

	int		width	= LOWORD(size);
	int		height	= HIWORD(size);
	
	OrthoMode(0,0,width, height);	
	
	float x = IMAGE_ICON_SPACEING;
	float y = IMAGE_ICON_SPACEING;
	float maxy = y;
	float tw;
	float th;

	
	for (int i=0; i<numtextures; i++) {
		glBindTexture(GL_TEXTURE_2D, texture[i].glID);

		tw = texture[i].width * imageiconscale;
		th = texture[i].height * imageiconscale;

		if (x+tw > width) {
			x = IMAGE_ICON_SPACEING;
			y = maxy;
		}		
	
		glColor3f(1,1,1);
		glEnable(GL_TEXTURE_2D);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex2f(x, y - SCROLL_SIZE*texturewindowscroll);

			glTexCoord2f(1,0);
			glVertex2f(x+tw, y - SCROLL_SIZE*texturewindowscroll);

			glTexCoord2f(1,1);
			glVertex2f(x+tw, y+th - SCROLL_SIZE*texturewindowscroll);

			glTexCoord2f(0,1);
			glVertex2f(x, y+th - SCROLL_SIZE*texturewindowscroll);

			
		glEnd();

		if (i==selectedtexture) {
			glColor3f(1,0,0);
			glDisable(GL_TEXTURE_2D);
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			glBegin(GL_QUADS);
				glVertex2f(x, y - SCROLL_SIZE*texturewindowscroll);
				glVertex2f(x+tw, y - SCROLL_SIZE*texturewindowscroll);
				glVertex2f(x+tw, y+th - SCROLL_SIZE*texturewindowscroll);				
				glVertex2f(x, y+th - SCROLL_SIZE*texturewindowscroll);
			glEnd();
		}

		x = x + tw + IMAGE_ICON_SPACEING;

		if (y + th + IMAGE_ICON_SPACEING> maxy)
			maxy = y + th + IMAGE_ICON_SPACEING;

		if (x > width) {
			x = IMAGE_ICON_SPACEING;
			y = maxy;
		}
	}

	PerspectiveMode();
	glwindow.PaintFrame();
}



/************************************************************************************


					CALLBACK FUNCTIONS


*************************************************************************************/


LRESULT CALLBACK WP_ProjectWin(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int xo;
	static int yo;
	POINT	   point;
	bool	   state;

	GLWindow *window = g_editor.GetGLWindow();


	switch (uMsg) {
		case WM_SIZE: {
			window->MakeCurrent(PROJECTIONWINDOW);
			window->ResizeOpenGLScreen();
			break;
		}
		case WM_LBUTTONDOWN: {

			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			state = true;
			window->SetInfo(SI_CMOUSELB, &state, PROJECTIONWINDOW);
			window->SetInfo(SI_CLDRAG, NULL, PROJECTIONWINDOW);

			break;
		}
		case WM_MBUTTONDOWN: {
			
			state = true;
			window->SetInfo(SI_CMOUSEMB, &state, PROJECTIONWINDOW);
			
			break;
		}
		case WM_RBUTTONDOWN: {

			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			state = true;
			window->SetInfo(SI_CMOUSERB, &state, PROJECTIONWINDOW);

			break;
		}
		case WM_LBUTTONUP: {

			state = false;
			window->SetInfo(SI_CMOUSELB, &state, PROJECTIONWINDOW);

			break;
		}
		case WM_MBUTTONUP: {

			state = false;
			window->SetInfo(SI_CMOUSEMB, &state, PROJECTIONWINDOW);

			break;
		}
		case WM_RBUTTONUP: {

			state = false;
			window->SetInfo(SI_CMOUSERB, &state, PROJECTIONWINDOW);

			break;
		}
		case WM_MOUSEMOVE: {

			if (wParam==MK_RBUTTON) {
				
				point = *((POINT *)window->GetInfo(GI_CRDRAG, PROJECTIONWINDOW));

				point.x += LOWORD(lParam) - xo;
				point.y += HIWORD(lParam) - yo;
				
				window->SetInfo(SI_CRDRAG, (void *)(&point), PROJECTIONWINDOW);
			}

			if (wParam==MK_LBUTTON) {
				
				point = *((POINT *)window->GetInfo(GI_CLDRAG, PROJECTIONWINDOW));

				point.x += LOWORD(lParam) - xo;
				point.y += HIWORD(lParam) - yo;

				window->SetInfo(SI_CLDRAG, (void *)(&point), PROJECTIONWINDOW);
			}

			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			point.x = xo;
			point.y = yo;
			
			window->SetInfo(SI_CMOUSEPOS, (void *)(&point), PROJECTIONWINDOW);


			break;
		}		

	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


LRESULT CALLBACK WP_ModellWin(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int xo;
	static int yo;
	POINT	   point;
	bool	   state;

	GLWindow *window = g_editor.GetGLWindow();

	switch (uMsg) {
		case WM_SIZE: {
			window->MakeCurrent(MODELLWINDOW);
			window->ResizeOpenGLScreen();
			break;
		}
		case WM_LBUTTONDOWN: {

			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			state = true;
			window->SetInfo(SI_CMOUSELB, &state, MODELLWINDOW);
			window->SetInfo(SI_CLDRAG, NULL, MODELLWINDOW);

			break;
		}
		case WM_MBUTTONDOWN: {
			
			state = true;
			window->SetInfo(SI_CMOUSEMB, &state, MODELLWINDOW);
			
			break;
		}
		case WM_RBUTTONDOWN: {

			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			state = true;
			window->SetInfo(SI_CMOUSERB, &state, MODELLWINDOW);

			break;
		}
		case WM_LBUTTONUP: {

			state = false;
			window->SetInfo(SI_CMOUSELB, &state, MODELLWINDOW);

			break;
		}
		case WM_MBUTTONUP: {

			state = false;
			window->SetInfo(SI_CMOUSEMB, &state, MODELLWINDOW);

			break;
		}
		case WM_RBUTTONUP: {

			state = false;
			window->SetInfo(SI_CMOUSERB, &state, MODELLWINDOW);

			break;
		}
		case WM_MOUSEMOVE: {

			if (wParam==MK_RBUTTON) {
				
				point = *((POINT *)window->GetInfo(GI_CRDRAG, MODELLWINDOW));

				point.x += LOWORD(lParam) - xo;
				point.y += HIWORD(lParam) - yo;
				
				window->SetInfo(SI_CRDRAG, (void *)(&point), MODELLWINDOW);
			}

			if (wParam==MK_LBUTTON) {
				
				point = *((POINT *)window->GetInfo(GI_CLDRAG, MODELLWINDOW));

				point.x += LOWORD(lParam) - xo;
				point.y += HIWORD(lParam) - yo;

				window->SetInfo(SI_CLDRAG, (void *)(&point), MODELLWINDOW);
			}

			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			point.x = xo;
			point.y = yo;
			
			window->SetInfo(SI_CMOUSEPOS, (void *)(&point), MODELLWINDOW);

			break;
		}
		case WM_MOUSEWHEEL:	{

			break;
		}

	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}




LRESULT CALLBACK WP_TextureWin(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int	scroll;
		
	switch (uMsg) {
		case WM_CREATE: {
			SCROLLINFO	si;

			scroll = 0;

			si.cbSize = sizeof(SCROLLINFO);
			si.nMin = 0;
			si.nMax = 0;
			si.nPage = 1;
			si.fMask = SIF_PAGE | SIF_RANGE;
			SetScrollInfo(hWnd, SB_VERT, &si, false);
			
			break;
		}
		case WM_VSCROLL: {
			SCROLLINFO	si;

			switch(LOWORD(wParam))
			{
				case SB_LINEUP:
					scroll--;
					break;
				case SB_PAGEUP:
					scroll--;
					break;
				case SB_LINEDOWN:
					scroll++;
					break;
				case SB_PAGEDOWN:
					scroll++;
					break;
				case SB_THUMBPOSITION:
					scroll = HIWORD(wParam);
					break;
			}

			si.cbSize = sizeof(SCROLLINFO);
			si.fMask = SIF_POS | SIF_RANGE;
			GetScrollInfo(hWnd, SB_VERT, &si);

			if(scroll != si.nPos)
			{
				if(scroll > si.nMax) scroll = si.nMax;
				else if(scroll < si.nMin) scroll = si.nMin;

				si.nPos = scroll;
				SetScrollInfo(hWnd, SB_VERT, &si, true);
			}

			g_editor.SetTextureWindowScroll(scroll);
			g_editor.DrawTexture();
			break;
		}
		case WM_SIZE: {
			GLWindow *window = g_editor.GetGLWindow();

			window->MakeCurrent(TEXTUREWINDOW);
			window->ResizeOpenGLScreen();

			if (g_editor.IsInitialized()) 
				g_editor.CalculateTextureScroll();

			break;
		}
		case WM_LBUTTONDOWN: {
			g_editor.PickTexture(LOWORD(lParam), HIWORD(lParam));
			break;
		}
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}



LRESULT CALLBACK WP_OneDWin(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int xo;
	static int yo;
	POINT	   point;
	bool	   state;

	GLWindow *window = g_editor.GetGLWindow();

	switch (uMsg) {
		case WM_SIZE: {
			window->MakeCurrent(ONEDWINDOW);
			window->ResizeOpenGLScreen();
			break;
		}
		case WM_LBUTTONDOWN: {

			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			state = true;
			window->SetInfo(SI_CMOUSELB, &state, ONEDWINDOW);
			window->SetInfo(SI_CLDRAG, NULL, ONEDWINDOW);

			break;
		}
		case WM_MBUTTONDOWN: {
			
			state = true;
			window->SetInfo(SI_CMOUSEMB, &state, ONEDWINDOW);
			
			break;
		}
		case WM_RBUTTONDOWN: {

			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			state = true;
			window->SetInfo(SI_CMOUSERB, &state, ONEDWINDOW);

			break;
		}
		case WM_LBUTTONUP: {

			state = false;
			window->SetInfo(SI_CMOUSELB, &state, ONEDWINDOW);

			break;
		}
		case WM_MBUTTONUP: {

			state = false;
			window->SetInfo(SI_CMOUSEMB, &state, ONEDWINDOW);

			break;
		}
		case WM_RBUTTONUP: {

			state = false;
			window->SetInfo(SI_CMOUSERB, &state, ONEDWINDOW);

			break;
		}
		case WM_MOUSEMOVE: {

			if (wParam==MK_RBUTTON) {
				
				point = *((POINT *)window->GetInfo(GI_CRDRAG, ONEDWINDOW));

				point.x += LOWORD(lParam) - xo;
				point.y += HIWORD(lParam) - yo;
				
				window->SetInfo(SI_CRDRAG, (void *)(&point), ONEDWINDOW);
			}

			if (wParam==MK_LBUTTON) {
				
				point = *((POINT *)window->GetInfo(GI_CLDRAG, ONEDWINDOW));

				point.x += LOWORD(lParam) - xo;
				point.y += HIWORD(lParam) - yo;

				window->SetInfo(SI_CLDRAG, (void *)(&point), ONEDWINDOW);
			}

			xo = LOWORD(lParam);
			yo = HIWORD(lParam);

			point.x = xo;
			point.y = yo;
			
			window->SetInfo(SI_CMOUSEPOS, (void *)(&point), ONEDWINDOW);


			break;
		}
		case WM_MOUSEWHEEL:	{

			break;
		}

	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


// filling the in the file dialog's properties
void FillParams(BROWSEINFO &bi, HWND hWnd, char *title, char *dir_name)
{
	bi.hwndOwner = hWnd;
	bi.pidlRoot = NULL;
	bi.pszDisplayName = dir_name;
	bi.lpszTitle = title;
	bi.ulFlags = BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
}


void FillOpenParams(OPENFILENAME &open_params, HWND hwnd, char *filter, char *file_name)
{
	open_params.lStructSize = sizeof(OPENFILENAME);
	open_params.hwndOwner = hwnd;
	open_params.lpstrFilter = filter;
	open_params.lpstrFile = file_name;
	open_params.nMaxFile = MAX_PATH;
	open_params.lpstrInitialDir = NULL; 
	open_params.lpstrFileTitle = NULL; 
	open_params.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_NOCHANGEDIR
						| OFN_HIDEREADONLY;
}


// main window's event handling is done here
LRESULT CALLBACK WndProc(HWND hWnd,UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) { 
		case WM_CREATE:
			break;
		case WM_PAINT:
			/*
			if (g_editor.IsInitialized()) {
				g_editor.Update();
			}
			*/
			break;
		case WM_SIZE:
			RECT rc;
			GetClientRect(hWnd, &rc);
			EnumChildWindows(hWnd, EnumFunc, (LPARAM)(&rc));
			break;
		case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
				case ID_TEXTURES_LOADFROMFILE: {
					DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LOAD_CATALOG), NULL, (DLGPROC)DlgProc_LoadCatalog);
					break;
				}
				case ID_FILE_OPEN1: {
					char filter[MAX_PATH] = {0};
					OPENFILENAME open_params = {0};
					char file_name[MAX_PATH] = {0};
					strcat(filter,"3D Files");
					int index = strlen(filter) + 1;

					filter[index++] = '*';
					filter[index++] = '.';
					filter[index++] = '3';
					filter[index++] = 'D';
					filter[index++] = 'W';
				
					FillOpenParams(open_params,hWnd,filter,file_name);

					if(GetOpenFileName(&open_params)) {
						if (strstr(open_params.lpstrFile, ".3dw")==NULL) {
							open_params.lpstrFile = strcat(open_params.lpstrFile, ".3dw");
						} 

						g_editor.PrintLog(open_params.lpstrFile);
						g_editor.LoadFile(open_params.lpstrFile);
					}
					

					break;
				}
				case ID_FILE_SAVE1: {
					char filter[MAX_PATH] = {0};
					OPENFILENAME open_params = {0};
					char file_name[MAX_PATH] = {0};
					strcat(filter,"3D Files");
					int index = strlen(filter) + 1;

					filter[index++] = '*';
					filter[index++] = '.';
					filter[index++] = '3';
					filter[index++] = 'D';
					filter[index++] = 'W';
				
					FillOpenParams(open_params,hWnd,filter,file_name);

					if(GetSaveFileName(&open_params)) {
						if (strstr(open_params.lpstrFile, ".3dw")==NULL) {
							open_params.lpstrFile = strcat(open_params.lpstrFile, ".3dw");
						} 

						g_editor.PrintLog(open_params.lpstrFile);
						g_editor.SaveFile(open_params.lpstrFile);
					}
					

					break;
				}
				case ID_VIEW_GLPOINT:
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_GLPOINT, MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_GLLINE, MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_GLFILL, MF_UNCHECKED);
					g_editor.SetRenderMode(RM_POINT);
					break;
				case ID_VIEW_GLLINE:
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_GLPOINT, MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_GLLINE, MF_CHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_GLFILL, MF_UNCHECKED);
					g_editor.SetRenderMode(RM_LINE);
					break;
				case ID_VIEW_GLFILL:
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_GLPOINT, MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_GLLINE, MF_UNCHECKED);
					CheckMenuItem(GetMenu(hWnd), ID_VIEW_GLFILL, MF_CHECKED);
					g_editor.SetRenderMode(RM_FILL);
					break;
				case ID_VIEW_TOP:
					g_editor.SetViewedFrom(TOP);					
					break;
				case ID_VIEW_FRONT:
					g_editor.SetViewedFrom(FRONT);
					break;
				case ID_VIEW_LEFT:
					g_editor.SetViewedFrom(LEFT);
					break;
				case ID_VIEW_BULBS:
					DWORD state;
					state = CheckMenuItem(GetMenu(hWnd), ID_VIEW_BULBS, MF_UNCHECKED);
					g_editor.SetShowBulbs(!state);
					break;
				case ID_CREATE_BEVEL:
					g_editor.CreateObject(ST_BEVEL);
					break;
				case ID_CREATE_ENDCAP:
					g_editor.CreateObject(ST_ENDCAP);
					break;
				case ID_CREATE_SIMPLEMESH:
					g_editor.CreateObject(ST_NPLANE);
					break;
				case ID_CREATE_CONE:
					g_editor.CreateObject(ST_NCONE);
					break;
				case ID_CREATE_CILINDER:
					g_editor.CreateObject(ST_NCILINDER);
					break;
				case ID_CREATE_SPHERE:
					g_editor.CreateObject(ST_SPHERE);
					break;
				case ID_CREATE_CAPS_BEVEL:
					g_editor.CreateObject(ST_IBCAP);
					break;
				case ID_CREATE_CAPS_ENDCAP:
					g_editor.CreateObject(ST_IECAP);
					break;
				case ID_CREATE_CAPS_INVERTEDBEVEL:
					g_editor.CreateObject(ST_BCAP);
					break;
				case ID_CREATE_CAPS_INVERTEDENDCAP:
					g_editor.CreateObject(ST_ECAP);
					break;					
				case ID_LIGHTS_GENERATE_LIGHTMAPS:
					g_editor.GenerateLightMaps();
					break;
				case ID_EDIT_DUPLICATE:
					g_editor.DuplicateSelectedObject();
					break;
				case ID_EDIT_DELETE:
					g_editor.DeleteSelectedObject();
					break;
				case ID_EDIT_INSERTOBJECT: {
					char filter[MAX_PATH] = {0};
					OPENFILENAME open_params = {0};
					char file_name[MAX_PATH] = {0};
					strcat(filter,"3D Files");
					int index = strlen(filter) + 1;

					filter[index++] = '*';
					filter[index++] = '.';
					filter[index++] = '3';
					filter[index++] = 'D';
					filter[index++] = 'W';
				
					FillOpenParams(open_params,hWnd,filter,file_name);

					if(GetOpenFileName(&open_params)) {
						if (strstr(open_params.lpstrFile, ".3dw")==NULL) {
							open_params.lpstrFile = strcat(open_params.lpstrFile, ".3dw");
						} 

						g_editor.PrintLog(open_params.lpstrFile);
						g_editor.AppendObject(open_params.lpstrFile);
					}

					break;
				}
				case ID_VIEW_SHOWLIGHTMAPS: {
					DWORD state;
					state = CheckMenuItem(GetMenu(hWnd), ID_VIEW_SHOWLIGHTMAPS, MF_UNCHECKED);
					g_editor.SetShowLightmaps(!state);
					break;
				}
				case ID_LIGHTS_INSERTLIGHT:
					g_editor.InsertLight();
					break;
				case ID_LIGHTS_MODIFYSELECTEDLIGHT:
					DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_LIGHT_PROPERTIES), NULL, (DLGPROC)DlgProc_LightProp);
					break;
				case ID_SELECT_SELECTINSIDE:
					g_editor.SelectInside();
					break;					
				case ID_TEXTURES_LOCKTEXTURE_ROTATIONS: {
					DWORD state;
					state = CheckMenuItem(GetMenu(hWnd), ID_TEXTURES_LOCKTEXTURE_ROTATIONS, MF_UNCHECKED);
					g_editor.LockTextureRotation(!state);
					break;
				}
				case ID_TEXTURES_LOCKTEXTURE_MOVES: {
					DWORD state;
					state = CheckMenuItem(GetMenu(hWnd), ID_TEXTURES_LOCKTEXTURE_MOVES, MF_UNCHECKED);
					g_editor.LockTextureMove(!state);
					break;
				}
				case ID_FILE_EXIT:
					PostQuitMessage(0);
					break;
				case ID_FILE_RESET:
					g_editor.Reset();
					break;
				case ID_TEXTURES_TEXTURINGPROPERTIES: {
					DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TEXTURE_PROPERTIES), NULL, (DLGPROC)DlgProc_TexProp);
					break;
				}
				case ID_OBJECT_SMOOTHNESS: {
					DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_SMOOTHNESS), NULL, (DLGPROC)DlgProc_Smoothness);
					break;
				}
				case ID_EDIT_ROTATE: {
					DWORD state;
					state = CheckMenuItem(GetMenu(hWnd), ID_EDIT_ROTATE, MF_UNCHECKED);
					
					if (state) g_editor.Rotation(false);
					else if (g_editor.Rotation(true)) CheckMenuItem(GetMenu(hWnd), ID_EDIT_ROTATE, MF_CHECKED);

					break;
				}
				case ID_EDIT_SCALEOBJECTS: {
					DWORD state;
					state = CheckMenuItem(GetMenu(hWnd), ID_EDIT_SCALEOBJECTS, MF_UNCHECKED);
					
					if (state) g_editor.EnableScaleObjects(false);
					else if (g_editor.EnableScaleObjects(true)) CheckMenuItem(GetMenu(hWnd), ID_EDIT_SCALEOBJECTS, MF_CHECKED);

					break;
				}
				case ID_EDIT_SNAPTOGRID: {
					DWORD state;
					state = CheckMenuItem(GetMenu(hWnd), ID_EDIT_SNAPTOGRID, MF_UNCHECKED);
					
					g_editor.SnapToGrid(!state);

					break;
				}
				case ID_SELECT_DRAGVERTICES: {
					DWORD state;
					state = CheckMenuItem(GetMenu(hWnd), ID_SELECT_DRAGVERTICES, MF_UNCHECKED);
					
					if (state) g_editor.DragVertices(false);
					else if (g_editor.DragVertices(true)) CheckMenuItem(GetMenu(hWnd), ID_SELECT_DRAGVERTICES, MF_CHECKED);

					break;
				}
			}	
			
			break;
		}
		case WM_MOUSEWHEEL:	{
			GLWindow	*window = g_editor.GetGLWindow();
			POINT		point = *((POINT *)window->GetInfo(GI_CRDRAG, PROJECTIONWINDOW));

			point.y += ((short)HIWORD(wParam))/6;

			window->SetInfo(SI_CRDRAG, (void *)(&point), PROJECTIONWINDOW);
			break;
		}
		case WM_SYSCOMMAND:						
		{
			switch (wParam)						
			{
				case SC_SCREENSAVE:				
				case SC_MONITORPOWER:			
				return 0;					
			}
			break;							
		}			
		case WM_KEYDOWN:
			switch(wParam) {
				case VK_ESCAPE:
					g_editor.BackStatus();
					break;
				case VK_UP:
					g_editor.camera.Forward(CAMERA_STEP);
					break;
				case VK_DOWN:
					g_editor.camera.Backward(CAMERA_STEP);
					break;
				case VK_LEFT:
					g_editor.camera.Left(CAMERA_STEP);
					break;
				case VK_RIGHT:
					g_editor.camera.Right(CAMERA_STEP);
					break;
				case VK_ADD:
					g_editor.ScaleMap(2.0f);
					break;
				case VK_SUBTRACT:
					g_editor.ScaleMap(0.5f);
					break;
				case VK_F2:
					SendMessage(hWnd, WM_COMMAND, ID_FILE_SAVE1, lParam);
					break;
				case VK_F5:
					SendMessage(hWnd, WM_COMMAND, ID_EDIT_ROTATE, lParam);
					break;
				case VK_F6:
					SendMessage(hWnd, WM_COMMAND, ID_SELECT_DRAGVERTICES, lParam);
					break;
				case VK_F7:
					SendMessage(hWnd, WM_COMMAND, ID_EDIT_SCALEOBJECTS, lParam);
					break;
				case VK_INSERT:
					SendMessage(hWnd, WM_COMMAND, ID_EDIT_DUPLICATE, lParam);
					break;
				case VK_DELETE:
					SendMessage(hWnd, WM_COMMAND, ID_EDIT_DELETE, lParam);
					break;
				case VK_RETURN:
					SendMessage(hWnd, WM_COMMAND, ID_SELECT_SELECTINSIDE, lParam);
					break;
				case VK_HOME:
					if (GetKeyState(VK_SHIFT) & SHIFTED) g_editor.ScaleTexture(0.1f, 0.0f);
					else g_editor.ShiftTexture(0.1f, 0.0f);
					break;
				case VK_END:
					if (GetKeyState(VK_SHIFT) & SHIFTED) g_editor.ScaleTexture(-0.1f, 0.0f);
					else g_editor.ShiftTexture(-0.1f, 0.0f);
					break;
				case VK_PRIOR:
					if (GetKeyState(VK_SHIFT) & SHIFTED) g_editor.ScaleTexture(0.0f, 0.1f);
					else g_editor.ShiftTexture(0.0f, 0.1f);
					break;
				case VK_NEXT:
					if (GetKeyState(VK_SHIFT) & SHIFTED) g_editor.ScaleTexture(0.0f, -0.1f);
					else g_editor.ShiftTexture(0.0f, -0.1f);
					break;
			}
			break;
		case WM_DESTROY:
		case WM_CLOSE:
			PostQuitMessage(0);
			break;		
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


// function for resizeing the child windows
BOOL CALLBACK EnumFunc(HWND hwnd, LPARAM lParam) {
	LPRECT rc; 
    int cID; 
 
    cID = GetWindowLong(hwnd, GWL_ID);
	cID-= FIRST_CHILD_WINDOW_ID;
 
    
    // Size and position the child window.  
    rc = (LPRECT) lParam;

	int width = rc->right - rc->left;
	int height = rc->bottom - rc->top - STATUS_HEIGHT;

    MoveWindow(hwnd,
			   (int)(g_rect[cID].x * width),
			   (int)(g_rect[cID].y * height),
			   (int)(g_rect[cID].width * width),
			   (int)(g_rect[cID].height * height),
			   TRUE);

	ShowWindow(hwnd, SW_SHOW); 
	
	return true;
}


LRESULT CALLBACK DlgProc_TexProp(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HWND VS, HS, W, H, T, B;

	if (uMsg==WM_INITDIALOG || uMsg==WM_COMMAND)
	{
		VS=GetDlgItem(hDlg, IDC_VS);
		HS=GetDlgItem(hDlg, IDC_HS);
		W=GetDlgItem(hDlg, IDC_W);
		H=GetDlgItem(hDlg, IDC_H);
		T=GetDlgItem(hDlg, IDC_T);
		B=GetDlgItem(hDlg, IDC_BILLBOARD);
	}

    switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			char text[MAX_PATH];
			CVector2 size, shift;
			float tr;
			bool  bb;

			if (!g_editor.GetTextureInfo(size, shift,tr,bb)) {
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}

			sprintf(text, "%f", shift.x);
			Edit_SetText(HS, text);
			sprintf(text, "%f", shift.y);
			Edit_SetText(VS, text);
			sprintf(text, "%f", size.x);
			Edit_SetText(W, text);
			sprintf(text, "%f", size.y);
			Edit_SetText(H, text);
			sprintf(text, "%f", tr);
			Edit_SetText(T, text);
			
			SendDlgItemMessage(hDlg, IDC_BILLBOARD, BM_SETCHECK, bb ? BST_CHECKED:BST_UNCHECKED, 0);
			
			return TRUE;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
				case IDC_OK1:
				{
					char text[MAX_PATH];
					float vs, hs, w,h, t;
					bool bb;
	
					Edit_GetText(VS, text, MAX_PATH);
					sscanf(text, "%f", &vs);
					Edit_GetText(HS, text, MAX_PATH);
					sscanf(text, "%f", &hs);
					Edit_GetText(W, text, MAX_PATH);
					sscanf(text, "%f", &w);
					Edit_GetText(H, text, MAX_PATH);
					sscanf(text, "%f", &h);
					Edit_GetText(T, text, MAX_PATH);
					sscanf(text, "%f", &t);
					bb = SendDlgItemMessage(hDlg, IDC_BILLBOARD, BM_GETSTATE, 0, 0); 

					if (t<0.0f) t=0.0f;
					if (t>1.0f) t=1.0f;

					g_editor.SetTextureInfo(CVector2(w, h), CVector2(hs,vs), t, bb);
					EndDialog(hDlg, LOWORD(wParam));

					return TRUE;
				}				
				case IDC_CANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
			}
			break;
		}
		case WM_DESTROY:
		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
	}
    return FALSE;
}


LRESULT CALLBACK DlgProc_LightProp(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HWND RED, GREEN, BLUE, X, Y, Z, R, B, C;

	if (uMsg==WM_INITDIALOG || uMsg==WM_COMMAND)
	{
		RED=GetDlgItem(hDlg, IDC_RED);
		GREEN=GetDlgItem(hDlg, IDC_GREEN);
		BLUE=GetDlgItem(hDlg, IDC_BLUE);
		X=GetDlgItem(hDlg, IDC_X);
		Y=GetDlgItem(hDlg, IDC_Y);
		Z=GetDlgItem(hDlg, IDC_Z);
		R=GetDlgItem(hDlg, IDC_R);
		B=GetDlgItem(hDlg, IDC_B);
		C=GetDlgItem(hDlg, IDC_C);
	}

    switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			char text[MAX_PATH];
			BULB b;
			float tr;

			if (!g_editor.GetLightInfo(b)) {
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}

			sprintf(text, "%3.0f", b.r);
			Edit_SetText(RED, text);
			sprintf(text, "%3.0f", b.g);
			Edit_SetText(GREEN, text);
			sprintf(text, "%3.0f", b.b);
			Edit_SetText(BLUE, text);
			sprintf(text, "%3.2f", b.pos.x);
			Edit_SetText(X, text);
			sprintf(text, "%3.2f", b.pos.y);
			Edit_SetText(Y, text);
			sprintf(text, "%3.2f", b.pos.z);
			Edit_SetText(Z, text);
			sprintf(text, "%1.4f", b.brightness);
			Edit_SetText(B, text);
			sprintf(text, "%1.4f", b.cutoff);
			Edit_SetText(C, text);
			sprintf(text, "%3.2f", b.range);
			Edit_SetText(R, text);

			return TRUE;
		}

		case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
				case IDC_OK:
				{
					char text[MAX_PATH];
					BULB b;

					Edit_GetText(RED, text, MAX_PATH);
					sscanf(text, "%f", &b.r);
					Edit_GetText(GREEN, text, MAX_PATH);
					sscanf(text, "%f", &b.g);
					Edit_GetText(BLUE, text, MAX_PATH);
					sscanf(text, "%f", &b.b);
					Edit_GetText(X, text, MAX_PATH);
					sscanf(text, "%f", &b.pos.x);
					Edit_GetText(Y, text, MAX_PATH);
					sscanf(text, "%f", &b.pos.y);
					Edit_GetText(Z, text, MAX_PATH);
					sscanf(text, "%f", &b.pos.z);
					Edit_GetText(B, text, MAX_PATH);
					sscanf(text, "%f", &b.brightness);
					Edit_GetText(C, text, MAX_PATH);
					sscanf(text, "%f", &b.cutoff);
					Edit_GetText(R, text, MAX_PATH);
					sscanf(text, "%f", &b.range);

					if (b.cutoff<0.0f) b.cutoff = 0.0f;
					if (b.cutoff>1.0f) b.cutoff = 1.0f;
					if (b.brightness<0.0f) b.brightness = 0.0f;
					if (b.brightness>1.0f) b.brightness = 1.0f;
					if (b.range<1.0f) b.range = 1.0f;
					if (b.r<0.0f)	b.r = 0.0f;
					if (b.r>255.0f) b.r = 255.0f;
					if (b.g<0.0f)	b.g = 0.0f;
					if (b.g>255.0f) b.g = 255.0f;
					if (b.b<0.0f)	b.b = 0.0f;
					if (b.b>255.0f) b.b = 255.0f;

					g_editor.SetLightInfo(b);

					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}				
				case IDC_CANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
			}
			break;
		}
		case WM_DESTROY:
		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
	}
    return FALSE;
}


LRESULT CALLBACK DlgProc_LoadCatalog(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {

    switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			WIN32_FIND_DATA FindFileData;
			HANDLE hFind;
			HWND list = GetDlgItem(hDlg, IDC_LIST);

			char fname[MAX_PATH];
			sprintf(fname,"textures/*");
			hFind = FindFirstFile(fname, &FindFileData);

			bool finish=false;

			if (hFind == INVALID_HANDLE_VALUE) return false;

			int i=0;

			while (!finish)
			{
				if (FindNextFile(hFind, &FindFileData))
				{
					if (FindFileData.dwFileAttributes==FILE_ATTRIBUTE_DIRECTORY &&
						strcmp("..", FindFileData.cFileName)!=0)
					{
						SendMessage(list, LB_ADDSTRING, 0, (LPARAM) FindFileData.cFileName); 
						SendMessage(list, LB_SETITEMDATA, i, (LPARAM) i);
						SetFocus(list);
						i++;
					}
				} else finish=true;
			}
			return TRUE;
		}
		case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
				case IDC_OK:
				{
					WIN32_FIND_DATA FindFileData;
					HANDLE hFind;
					char	fname[MAX_PATH];
					char	selected[MAX_PATH];
					HWND	list = GetDlgItem(hDlg, IDC_LIST);
					int		index = SendMessage(list, LB_GETCURSEL, 0, 0);
					
					ListBox_GetText(list, index, selected);
					
					sprintf(fname, "textures/%s/*.jpg", selected);
					hFind = FindFirstFile(fname, &FindFileData);

					g_editor.FlushTextures();

					bool finish=false;

					if (hFind != INVALID_HANDLE_VALUE) 
					{
						sprintf(fname,"textures/%s/%s", selected, FindFileData.cFileName);
						g_editor.AddTexture(fname);
					} else finish=true;	
					EndDialog(hDlg, LOWORD(wParam));

					while (!finish)
					{
						if (FindNextFile(hFind, &FindFileData))
						{
							sprintf(fname,"textures/%s/%s", selected, FindFileData.cFileName);
							g_editor.AddTexture(fname);
						} else finish=true;
					}

					g_editor.CalculateTextureScroll();
					return TRUE;
				}				
				case IDC_CANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
			}
			break;
		}
		case WM_DESTROY:
		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
	}
    return FALSE;
}


LRESULT CALLBACK DlgProc_Smoothness(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int scroll;
	
	
    switch (uMsg)
	{
		case WM_INITDIALOG:
		{

			if (!g_editor.GetSmoothness(scroll)) {
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}

			HWND ST, SB;
			ST = GetDlgItem(hDlg, IDC_ST);
			SB = GetDlgItem(hDlg, IDC_SCROLL);

			ScrollBar_SetRange(SB, MIN_SMOOTHNESS, MAX_SMOOTHNESS, true);
			ScrollBar_SetPos(SB, scroll, true);
			SetScrollPos(SB, SB_CTL, scroll, true);

			char text[MAX_NAME];

			sprintf(text, "%d", scroll);
			Static_SetText(ST, text);

			return TRUE;
		}
		case WM_HSCROLL: {
			switch(LOWORD(wParam))
			{
				case SB_LINELEFT:
					scroll--;
					break;
				case SB_LINERIGHT:
					scroll++;
					break;
				case SB_PAGERIGHT:
					scroll++;
					break;
				case SB_PAGELEFT:
					scroll--;
					break;
				case SB_THUMBPOSITION:
					scroll = HIWORD(wParam);
					break;
			}

			HWND ST, SB;
			ST = GetDlgItem(hDlg, IDC_ST);
			SB = GetDlgItem(hDlg, IDC_SCROLL);
			
			
			if(scroll != ScrollBar_GetPos(SB))
			{
				if(scroll > MAX_SMOOTHNESS) scroll = MAX_SMOOTHNESS;
				else if(scroll < MIN_SMOOTHNESS) scroll = MIN_SMOOTHNESS;
			}

			SetScrollPos(SB, SB_CTL, scroll, true);

			char text[MAX_NAME];

			sprintf(text, "%d", scroll);
			Static_SetText(ST, text);
			
			break;
		}
		
		case WM_COMMAND:
		{
			switch(LOWORD(wParam)) {
				case IDC_OK:
				{
					g_editor.SetSmoothness(scroll);
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}				
				case IDC_CANCEL:
				{
					EndDialog(hDlg, LOWORD(wParam));
					return TRUE;
				}
			}
			break;
		}
		case WM_DESTROY:
		case WM_CLOSE:
			EndDialog(hDlg, LOWORD(wParam));
			return TRUE;
	}
    return FALSE;
}