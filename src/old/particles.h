#ifndef PARTICLES_H
#define PARTICLES_H

#include <zfwc.h>
#include "sprites.h"

#define PARTICLE_LIMIT 1024

typedef struct {
    e_sprite spr; // NOTE: Not needed during tick!

    s_v2 pos;
    s_v2 vel;

    t_r32 rot;

    t_s32 life;
} s_particle;

typedef struct {
    s_particle buf[PARTICLE_LIMIT];
    t_s32 cnt;
} s_particles;

typedef enum {
    ek_particle_template_dirt,
    ek_particle_template_stone,
    ek_particle_template_grass,
    ek_particle_template_sand,
    ek_particle_template_gel,

    eks_particle_template_cnt
} e_particle_template;

void InitParticleFromTemplate(s_particle* const part, const e_particle_template temp, const s_v2 pos, const s_v2 vel, const t_r32 rot);
t_s32 AddParticle(s_particles* const particles, const s_particle* const part);
void UpdateParticles(s_particles* const particles, const t_r32 grav);
void RenderParticles(const s_particles* const particles, const s_rendering_context* const rendering_context, const s_texture_group* const textures);

static inline t_s32 SpawnParticleFromTemplate(s_particles* const particles, const e_particle_template temp, const s_v2 pos, const s_v2 vel, const t_r32 rot) {
    s_particle part = {0};
    InitParticleFromTemplate(&part, temp, pos, vel, rot);
    return AddParticle(particles, &part);
}

#endif
