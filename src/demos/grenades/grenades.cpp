/*
 * Demonstration of damping v non damping implementations
 *
 */

#include <cyclone/cyclone.h>
#include "../ogl_headers.h"
#include "../app.h"
#include "../timing.h"

#include <stdio.h>
#include <iostream>
const float degToRad = 3.141592653589793 / 180.0f;

class GrenadeDemo : public Application
{
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
	
	// define the player's heading.  i.e. the direction the projectile shoots	
	float heading = 90.0f;

	// create force registry for the objects
//	cyclone::ParticleForceRegistry registry;

//	cyclone::Vector3 center = cyclone::Vector3(50.0f, 0.0f, 50.0f);
//	cyclone::Vector3 updraft = cyclone::Vector3(0.0f, 5.0f, 0.0f);
//	cyclone::ParticleUplift pu = cyclone::ParticleUplift(center, 5.0f, updraft);
//	const double grav = 0.66742;
//	cyclone::Vector3 gravCenter = cyclone::Vector3(0.0f, 20.0f, 50.0f);
//	cyclone::ParticlePointGravity pG = cyclone::ParticlePointGravity(gravCenter, 100000.0f, 10.0f, grav);

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
		shot->particle.setVelocity(cos(heading*degToRad)*35.0f, 0.0f, sin(heading*degToRad)*35.0f);
		shot->particle.setAcceleration(0.0f,-1.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;

	case ARTILLERY:
		shot->particle.setMass(2000.0f);
		shot->particle.setVelocity(cos(heading*degToRad)*40.0f, 30.0f, sin(heading*degToRad)*40.0f);
		shot->particle.setAcceleration(0.0f, -20.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;

	case FIREBALL:
		shot->particle.setMass(1.0f);
		shot->particle.setVelocity(cos(heading*degToRad)*10.0f, 0.0f, sin(heading*degToRad)*10.0f);
		shot->particle.setAcceleration(0.0f, 0.6f, 0.0f);
		shot->particle.setDamping(0.9f);
		break;

	case LASER:
		shot->particle.setMass(0.1f);
		shot->particle.setVelocity(cos(heading*degToRad)*100.0f, 0.0f, sin(heading*degToRad)*100.0f);
		shot->particle.setAcceleration(0.0f, 0.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;

	case GRENADELAUNCHER:
		shot->particle.setMass(12.0f);
		shot->particle.setVelocity(cos(heading*degToRad)*10.0f, 10.0f, sin(heading*degToRad)*10.0f);
		shot->particle.setAcceleration(0.0f, -8.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;

	case GRENADE:
		shot->particle.setMass(12.0f);
		shot->particle.setVelocity(cos(heading*degToRad)*4.0f, 4.0f, sin(heading*degToRad)*4.0f);
		shot->particle.setAcceleration(0.0f, -3.0f, 0.0f);
		shot->particle.setDamping(0.99f);
		break;
	}

	// TODO: Set the particles initial position to be that of the players
	shot->particle.setPosition(0.0f, 1.5f, 0.0f);
	shot->startTime = TimingData::get().lastFrameTimestamp;
	shot->type = currentShotType;
	
//	registry.add(&shot->particle, &pu);
//	registry.add(&shot->particle, &pG);
	// zeros out the forces on the particle
	shot->particle.clearAccumulator();	
}

void GrenadeDemo::update()
{

	float duration = (float)TimingData::get().lastFrameDuration * 0.001f;
	if (duration <= 0.0f) return;

	for (AmmoRound *shot = ammo; shot < ammo+ammoRounds; shot++)
	{
		if (shot->type != UNUSED)
		{
//			registry.updateForces(duration);
			shot->particle.integrate(duration);
			if (shot->particle.getPosition().y < 0.0f ||
			shot->startTime+5000 < TimingData::get().lastFrameTimestamp ||
			shot->particle.getPosition().z > 200.0f)
			{	
				shot->type = UNUSED;
//				registry.remove(&shot->particle, &pu);
//				registry.remove(&shot->particle, &pG);
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

	glColor3f(0.75f, 0.75f, 0.75f);
	glBegin(GL_LINES);
	// players position
	glVertex3f(0.0f, 1.5f, 0.0f);
	glVertex3f(cos(heading*degToRad)*10.0f,1.5f, sin(heading*degToRad)*10.0f);
	glEnd();

	glColor3f(0.75f, 0.75f, 0.75f);
	glBegin(GL_LINE_LOOP);

	for (int i = 0; i < 360.0f; ++i)
	{
		glVertex3f(cos(i*degToRad)*10.0f, 1.5f, sin(i*degToRad)*10.0f);
	}
	glEnd();
	
	glColor3f(0.75f, 0.75f, 0.75f);
	glBegin(GL_LINE_LOOP);

	for (int i = 0; i < 360.0f; ++i)
	{
		glVertex3f(0.0f + cos(i*degToRad)*10.0f, 20.0f, 50.0+sin(i*degToRad)*10.0f);
	}
	glEnd();

	glColor3f(0.75f, 0.75f, 0.75f);
	glBegin(GL_LINE_LOOP);
	for (int i = 0; i < 360.0f; ++i) {
		glVertex3f(50.0f + cos(i*degToRad)*5.0f, 0.0f, 50.0f + sin(i*degToRad)*5.0f);
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
	case 'a': --heading; break;
	case 'd': ++heading; break;
	case 'A': heading -= 2; break;
	case 'D': heading += 2; break;
	}
}

Application* getApplication()
{
	return new GrenadeDemo();
}
