/*
 * Damping Demo
 */

#include <cyclone/cyclone.h>
#include "../ogl_headers.h"
#include "../app.h"
#include "../timing.h"

#include <stdio.h>
#include <cstdlib>

class DampingDemo : public Application
{

	struct ParticleObject
	{
		cyclone::Particle particle;
		unsigned startTime;

		void render()
		{
			cyclone::Vector3 position;
			particle.getPosition(&position);

			glColor3f(0, 0, 0);
        		glPushMatrix();
        	    	glTranslatef(position.x, position.y, position.z);
        	    	glutSolidSphere(0.3f, 5, 4);
        	    	glPopMatrix();
	
        	    	glColor3f(0.75, 0.75, 0.75);
        	    	glPushMatrix();
        	    	glTranslatef(position.x, 0, position.z);
        	    	glScalef(1.0f, 0.1f, 1.0f);
        	    	glutSolidSphere(0.6f, 5, 4);
	            	glPopMatrix();
		}
	};

	const static unsigned numParticles = 2;

	ParticleObject particles[2];
	
	float frameRates[10] = {1.0/20.0, 1.0/30.0, 1.0/40.0, 1.0/50.0, 1.0/60.0, 1.0/70.0, 1.0/80.0, 1.0/90.0, 1.0/100.0, 1.0/110.0};

public:
	
	DampingDemo();

	virtual const char* getTitle();

	virtual void update();

	virtual void display();

};

DampingDemo::DampingDemo() 
{

}

const char* DampingDemo::getTitle()
{
	return "Cyclone >> Damping Demo";
}

void DampingDemo::update()
{
	// randomize these
	srand(nullptr);
	int index = rand()%10;

	float duration = frameRates[index];

	if (duration <= 0.0f) return;
	
	particles[0].integrate35Damping(duration);

	particles[1].integrate36Damping(duration);

	Application::update();
}
