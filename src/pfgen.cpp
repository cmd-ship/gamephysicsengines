/*
 * Implementation file for the particle force generators.
 *
 * Part of the Cyclone physics system.
 *
 * Copyright (c) Icosagon 2003. All Rights Reserved.
 *
 * This software is distributed under licence. Use of this software
 * implies agreement with all terms and conditions of the accompanying
 * software licence.
 */

#include <cyclone/pfgen.h>
#include <stdio.h>
#include <iostream>
#include <math.h>
using namespace cyclone;


void ParticleForceRegistry::updateForces(real duration)
{
    Registry::iterator i = registrations.begin();
    for (; i != registrations.end(); i++)
    {
        i->fg->updateForce(i->particle, duration);
    }
}

void ParticleForceRegistry::add(Particle* particle, ParticleForceGenerator *fg)
{
    ParticleForceRegistry::ParticleForceRegistration registration;
    registration.particle = particle;
    registration.fg = fg;
    registrations.push_back(registration);
}

void ParticleForceRegistry::remove(Particle* p, ParticleForceGenerator *f)
{
	int count = 0;
	bool found=false;
	for (Registry::iterator i = registrations.begin(); i != registrations.end(); ++i, ++count)
	{
		if (i->particle == p && i->fg == f){
			found = true;       
			break;
		}
	}
	if (found) registrations.erase(registrations.begin()+count);
}

ParticleWind::ParticleWind(const Vector3& wind) : wind(wind){}

void ParticleWind::updateForce(Particle* particle, real duration) 
{
	if (!particle->hasFiniteMass()) return;
	particle->addForce(wind);
}

void ParticleWind::updateWindSpeed(Vector3 w)
{
	wind = w;
}

ParticleGravity::ParticleGravity(const Vector3& gravity)
: gravity(gravity)
{
}

void ParticleGravity::updateForce(Particle* particle, real duration)
{
    // Check that we do not have infinite mass
    if (!particle->hasFiniteMass()) return;

    // Apply the mass-scaled force to the particle
    particle->addForce(gravity * particle->getMass());
}

ParticleDrag::ParticleDrag(real k1, real k2)
: k1(k1), k2(k2)
{
}

void ParticleDrag::updateForce(Particle* particle, real duration)
{
    Vector3 force;
    particle->getVelocity(&force);

    // Calculate the total drag coefficient
    real dragCoeff = force.magnitude();
    dragCoeff = k1 * dragCoeff + k2 * dragCoeff * dragCoeff;

    // Calculate the final force and apply it
    force.normalise();
    force *= -dragCoeff;
    particle->addForce(force);
}

ParticleSpring::ParticleSpring(Particle *other, real sc, real rl)
: other(other), springConstant(sc), restLength(rl)
{
}

void ParticleSpring::updateForce(Particle* particle, real duration)
{
    // Calculate the vector of the spring
    Vector3 force;
    particle->getPosition(&force);
    force -= other->getPosition();

    // Calculate the magnitude of the force
    real magnitude = force.magnitude();
    magnitude = real_abs(magnitude - restLength);
    magnitude *= springConstant;

    // Calculate the final force and apply it
    force.normalise();
    force *= -magnitude;
    particle->addForce(force);
}

ParticleBuoyancy::ParticleBuoyancy(real maxDepth,
                                 real volume,
                                 real waterHeight,
                                 real liquidDensity)
:
maxDepth(maxDepth), volume(volume),
waterHeight(waterHeight), liquidDensity(liquidDensity)
{
}

void ParticleBuoyancy::updateForce(Particle* particle, real duration)
{
    // Calculate the submersion depth
    real depth = particle->getPosition().y;

    // Check if we're out of the water
    if (depth >= waterHeight + maxDepth) return;
    Vector3 force(0,0,0);

    // Check if we're at maximum depth
    if (depth <= waterHeight - maxDepth)
    {
        force.y = liquidDensity * volume;
        particle->addForce(force);
        return;
    }

    // Otherwise we are partly submerged
    force.y = liquidDensity * volume *
        (depth - maxDepth - waterHeight) / 2 * maxDepth;
    particle->addForce(force);
}

ParticleBungee::ParticleBungee(Particle *other, real sc, real rl)
: other(other), springConstant(sc), restLength(rl)
{
}

void ParticleBungee::updateForce(Particle* particle, real duration)
{
    // Calculate the vector of the spring
    Vector3 force;
    particle->getPosition(&force);
    force -= other->getPosition();

    // Check if the bungee is compressed
    real magnitude = force.magnitude();
    if (magnitude <= restLength) return;

    // Calculate the magnitude of the force
    magnitude = springConstant * (restLength - magnitude);

    // Calculate the final force and apply it
    force.normalise();
    force *= -magnitude;
    particle->addForce(force);
}

ParticleFakeSpring::ParticleFakeSpring(Vector3 *anchor, real sc, real d)
: anchor(anchor), springConstant(sc), damping(d)
{
}

void ParticleFakeSpring::updateForce(Particle* particle, real duration)
{
    // Check that we do not have infinite mass
    if (!particle->hasFiniteMass()) return;

    // Calculate the relative position of the particle to the anchor
    Vector3 position;
    particle->getPosition(&position);
    position -= *anchor;

    // Calculate the constants and check they are in bounds.
    real gamma = 0.5f * real_sqrt(4 * springConstant - damping*damping);
    if (gamma == 0.0f) return;
    Vector3 c = position * (damping / (2.0f * gamma)) +
        particle->getVelocity() * (1.0f / gamma);

    // Calculate the target position
    Vector3 target = position * real_cos(gamma * duration) +
        c * real_sin(gamma * duration);
    target *= real_exp(-0.5f * duration * damping);

    // Calculate the resulting acceleration and therefore the force
    Vector3 accel = (target - position) * ((real)1.0 / (duration*duration)) -
        particle->getVelocity() * ((real)1.0/duration);
    particle->addForce(accel * particle->getMass());
}

ParticleAnchoredSpring::ParticleAnchoredSpring()
{
}

ParticleAnchoredSpring::ParticleAnchoredSpring(Vector3 *anchor,
                                               real sc, real rl)
: anchor(anchor), springConstant(sc), restLength(rl)
{
}

void ParticleAnchoredSpring::init(Vector3 *anchor, real springConstant,
                                  real restLength)
{
    ParticleAnchoredSpring::anchor = anchor;
    ParticleAnchoredSpring::springConstant = springConstant;
    ParticleAnchoredSpring::restLength = restLength;
}

void ParticleAnchoredBungee::updateForce(Particle* particle, real duration)
{
    // Calculate the vector of the spring
    Vector3 force;
    particle->getPosition(&force);
    force -= *anchor;

    // Calculate the magnitude of the force
    real magnitude = force.magnitude();
    if (magnitude < restLength) return;

    magnitude = magnitude - restLength;
    magnitude *= springConstant;

    // Calculate the final force and apply it
    force.normalise();
    force *= -magnitude;
    particle->addForce(force);
}

void ParticleAnchoredSpring::updateForce(Particle* particle, real duration)
{
    // Calculate the vector of the spring
    Vector3 force;
    particle->getPosition(&force);
    force -= *anchor;

    // Calculate the magnitude of the force
    real magnitude = force.magnitude();
    magnitude = (restLength - magnitude) * springConstant;

    // Calculate the final force and apply it
    force.normalise();
    force *= magnitude;
    particle->addForce(force);
}

ParticleUplift::ParticleUplift(Vector3 &centerPoint, real radius, Vector3 &Updraft) 
: centerPoint(centerPoint), radius(radius), Updraft(Updraft)
{
}

void ParticleUplift::init(Vector3 center, real r, Vector3 draftSpeed)
{
	ParticleUplift::centerPoint = center;
	ParticleUplift::radius = r;
	ParticleUplift::Updraft = draftSpeed;
}

void ParticleUplift::updateForce(Particle* particle, real duration) 
{
	real x = centerPoint.x - particle->getPosition().x;
	real z = centerPoint.z - particle->getPosition().z;
	real distance = real_sqrt(real_pow(x,2) + real_pow(z,2));
	//real distance = real_sqrt(distanceVector.magnitude());
	if(distance > radius) return;
	
	particle->addForce(Updraft);

}

ParticleAirBrake::ParticleAirBrake(real k1, real k2)
: k1(k1), k2(k2)
{
}
/**
 * 
 */
void ParticleAirBrake::updateForce(Particle* particle, real duration)
{
    if (!brakeOn) return;

    Vector3 velocity;
    particle->getVelocity(&velocity);
    
    // Calculate the total drag coefficient
    real dragCoeff = k1 * velocity.magnitude() + k2 * velocity.squareMagnitude();

    // Calculate the final force and apply it
    Vector3 force = velocity.unit() * -dragCoeff;
    particle->addForce(force);
}

/**
 * Toggles the airbrake on.  This applies to all particles
 */
void ParticleAirBrake::toggleBrake() {
	brakeOn = (brakeOn == false) ? true : false;
}

ParticlePointGravity::ParticlePointGravity(Vector3 &centerPoint, real radius, real mass, double g) 
: centerPoint(centerPoint), radius(radius), mass(mass), G(g)
{
}
/**
 * Measures the distance between the objects.  
 *
 * Square the scalar value of the distance vector.  
 * Since we are interested in the square of the distance - let's make it easy by canceling the sqrt and calling squareMagnitude()
 *
 * Compute the vector Fg = rHat * fg
 */
void ParticlePointGravity::updateForce(Particle* particle, real duration) 
{
	// distance between the objects
	Vector3 distance = centerPoint - particle->getPosition();
	// radius squared
	real distanceSquared = distance.squareMagnitude();
	// Calculate vector gforce
	Vector3 gForce = distance.unit() * (G * (mass*particle->getMass() / distanceSquared));
	
	particle->addForce(gForce);
}
