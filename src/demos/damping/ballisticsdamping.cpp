/*
 * Demonstration of damping v non damping implementations
 *
 */

#include <cyclone/cyclone.h>
#include "../ogl_headers.h"
#include "../app.h"
#include "../timing.h"

#include <stdio.h>

class GrenadeDemo : public Application
{

	float framerates[10] = {1.0/20.0f, 1.0/30.0f, 1.0/40.0f, 1.0/50.0f, 1.0/60.0f, 1.0/70.0, 1.0/80.0, 1.0/90.0f, 1.0/100.0f, 1.0/110.0f};

	enum ShotType
	{
		UNUSED = 0,
		PISTOL,
		ARTILLERY,
		FIREBALL,
		LASER,
		GRENADELAUNCHER,
		GRENADE
	};

	struct AmmoRound
	{
		cyclone::Particle particle;
		ShotType type;
		unsigned startTime;

		/* Draws the round */
		void render()
		{
			cyclone::Vector3 position;
			particle.getPosition(&position);

			glColor3f(0,0,0);
			glPushMatrix();
			glTranslatef(position.x, position.y, position.z);
			glutSolidSphere(0.3f, 5, 4);
			glPopMatrix();

			glColor3f(0.75,0.75,0.75);
			glPushMatrix();
			glTranslatef(position.x, 0, position.z);
			glScalef(1.0f, 0.1f, 1.0f);
			glutSolidSphere(0.6f, 5, 4);
			glPopMatrix();
		}
	};

	const static unsigned ammoRounds = 24;

	AmmoRound ammo[ammoRounds];

	ShotType currentShotType;

	void fire();

public:

	GrenadeDemo();

	virtual const char* getTitle();

	virtual void update();

	virtual void display();

	virtual void mouse(int button, int state, int x, int y);

	virtual void key(unsigned char key);
};

GrenadeDemo::GrenadeDemo()
: currentShotType(LASER)
{
	srand(0);
	for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; ++shot)
	{
		shot->type = UNUSED;
	}
}

const char* GrenadeDemo::getTitle()
{
	return "Cyclone >> Grenade Demo";
}

void GrenadeDemo::fire()
{
	
	AmmoRound *shot;
	for (shot = ammo; shot < ammo+ammoRounds; shot++)
	{
		if (shot->type == UNUSED) break;
	}

	if (shot >= ammo+ammoRounds) return;

	switch(currentShotType)
	{
	case PISTOL:
		shot->particle.setMass(2.0f);
		shot->particle.setVelocity(0.0f, 0.0f, 35.0f);
		shot->particle.setAcceleration(0.0f,-1.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;

	case ARTILLERY:
		shot->particle.setMass(200.0f);
		shot->particle.setVelocity(0.0f, 30.0f, 40.0f);
		shot->particle.setAcceleration(0.0f, -20.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;

	case FIREBALL:
		shot->particle.setMass(1.0f);
		shot->particle.setVelocity(0.0f, 0.0f, 10.0f);
		shot->particle.setAcceleration(0.0f, 0.6f, 0.0f);
		shot->particle.setDamping(0.9f);
		break;

	case LASER:
		shot->particle.setMass(0.1f);
		shot->particle.setVelocity(0.0f, 0.0f, 100.0f);
		shot->particle.setAcceleration(0.0f, 0.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;

	case GRENADELAUNCHER:
		shot->particle.setMass(12.0f);
		shot->particle.setVelocity(0.0f, 10.0f, 10.0f);
		shot->particle.setAcceleration(0.0f, -8.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;

	case GRENADE:
		shot->particle.setMass(12.0f);
		shot->particle.setVelocity(0.0f, 4.0f, 4.0f);
		shot->particle.setAcceleration(0.0f, -3.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;
	}

	shot->particle.setPosition(0.0f, 1.5f, 0.0f);
	shot->startTime = TimingData::get().lastFrameTimestamp;
	shot->type = currentShotType;

	shot->particle.clearAccumulator();	
}

void GrenadeDemo::update()
{
	int index = rand() % 10;
	float duration = framerates[index];
	if (duration <= 0.0f) return;
	int count = 0;
	for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++)
	{
		if (shot->type != UNUSED)
		{
			if (count % 2 == 0) {
				shot->particle.integrate36Damping(duration);
			} else {
				shot->particle.integrate35Damping(duration);
			}
			if (shot->particle.getPosition().y < 0.0f ||
			shot->startTime+5000 < TimingData::get().lastFrameTimestamp ||
			shot->particle.getPosition().z > 200.0f)
			{	
				shot->type = UNUSED;
			}
		}
	}
	Application::update();
}

void GrenadeDemo::display()
{

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
	gluLookAt(-25.0, 8.0, 5.0, 0.0, 5.0, 22.0, 0.0, 1.0, 0.0);

	glColor3f(0.0f, 0.0f, 0.0f);
	glPushMatrix();
	glTranslatef(0.0f, 1.5f, 0.0f);
	glutSolidSphere(0.1f, 5, 5);
	glTranslatef(0.0f, -1.5f, 0.0f);
	glColor3f(0.75f, 0.75f, 0.75f);
	glScalef(1.0f, 0.1f, 1.0f);
	glutSolidSphere(0.1f, 5, 5);
	glPopMatrix();

	glColor3f(0.75f, 0.75f, 0.75f);
	glBegin(GL_LINES);
	for (unsigned i = 0; i < 200; i += 10)
	{
		glVertex3f(-5.0f, 0.0f, i);
		glVertex3f(5.0f, 0.0f, i);
	}
	glEnd();

	for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++) {
		if (shot->type != UNUSED)
		{
			shot->render();
		}
	}

	glColor3f(0.0f, 0.0f, 0.0f);
	renderText(10.0f, 34.0f, "Click: Fire\n1-6: Select Ammo");

	switch(currentShotType)
	{
	case PISTOL: renderText(10.0f, 10.0f, "Current Ammo: Pistol"); break;
	case ARTILLERY: renderText(10.0f, 10.0f, "Current Ammo: Artillery"); break;
	case FIREBALL: renderText(10.0f, 10.0f, "Current Ammo: Fireball"); break;
	case LASER: renderText(10.0f, 10.0f, "Current Ammo: Laser"); break;
	case GRENADELAUNCHER: renderText(10.0f, 10.0f, "Current Ammo: Grenade Launcher"); break;
	case GRENADE: renderText(10.0f, 10.0f, "Current Ammo: Grenade"); break;
	}
}

void GrenadeDemo::mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) fire();
}

void GrenadeDemo::key(unsigned char key)
{
	switch(key)
	{
	case '1': currentShotType = PISTOL; break;
	case '2': currentShotType = ARTILLERY; break;
	case '3': currentShotType = FIREBALL; break;
	case '4': currentShotType = LASER; break;
	case '5': currentShotType = GRENADELAUNCHER; break;
	case '6': currentShotType = GRENADE; break;
	}
}

Application* getApplication()
{
	return new GrenadeDemo();
}
