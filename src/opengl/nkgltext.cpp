/*
* Stepa - OpenGL hex game
*
* (C) Copyright 2014
* Author: Nikola Knezevic <nkcodeplus@gmail.com>
*
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; version 2
* of the License.
*/

#include "nkgltext.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <GL/glew.h>
#include "include/framework.h"
using namespace std;
/**
* Render text from font
*/
NkGlText::NkGlText()
{
    face = 0;
}

NkGlText::~NkGlText()
{
    if (face != 0)
        delete face;
    glDeleteProgram(program);
}

int NkGlText::Init(int w, int h)
{
    m_duzina=w;
    m_visina=h;

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    fontfilename = "./data/font/FreeSans.ttf";
    face = new FT_Face();
    ft = new FT_Library();
    /* Initialize the FreeType2 library */
    if (FT_Init_FreeType(ft)) {
        fprintf(stderr, "Could not init freetype library\n");
        return 0;
    }

    /* Load a font */
    if (FT_New_Face(*ft, fontfilename, 0, face)) {
        fprintf(stderr, "Could not open font %s\n", fontfilename);
        return 0;
    }
    framework frm;
    program = frm.LoadShadersFromFile("./data/shaders/text.v.glsl", "./data/shaders/text.f.glsl");
    if(program == 0){
        std::cout<<"grska shader 1" << std::endl;
        return 0;
    }

    //attribute_coord = get_attrib(program, "coord");
    attribute_coord = glGetAttribLocation(program, "coord");
    uniform_tex = glGetUniformLocation(program, "tex");
    uniform_color = glGetUniformLocation(program, "color");

    if(attribute_coord == -1 || uniform_tex == -1 || uniform_color == -1){
        std::cout<<"grska shader" <<attribute_coord<< uniform_tex <<uniform_color<<  std::endl;
        return 0;
    }

    // Create the vertex buffer object
    glGenBuffers(1, &vbo);
    glUseProgram(program);
    /* Create texture atlasses for several font sizes */
    //a48 = new atlas(face, 48, uniform_tex);
    //a24 = new atlas(face, 24, uniform_tex);
    a12 = new atlas(face, 12, uniform_tex);
    glUseProgram(0);


    return 1;
}
void NkGlText::renderText(std::string msg,int x ,int y, glm::vec4 color){
    float sx = 2.0 / m_duzina;
    float sy = 2.0 / m_visina;

    glUseProgram(program);
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat boja[4] = { color[0], color[1], color[2], color[3] };

    /* Set color to black */
    glUniform4fv(uniform_color, 1, boja);
    render_text(msg.c_str(), a12, -1 + x * sx, 1 - y * sy, sx, sy);

    glBindVertexArray( 0 );
    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);

}
void NkGlText::renderClickMessage(string msg){
    float sx = 2.0 / m_duzina;
    float sy = 2.0 / m_visina;

    glUseProgram(program);
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat black[4] = { 0, 0, 0, 1 };
    //GLfloat red[4] = { 1, 0, 0, 1 };
    //GLfloat transparent_green[4] = { 0, 1, 0, 0.5 };

    /* Set color to black */
    glUniform4fv(uniform_color, 1, black);
    render_text(msg.c_str(), a12, -1 + 8 * sx, 1 - 20 * sy, sx, sy);

    glBindVertexArray( 0 );
    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);

}

void NkGlText::render()
{
    float sx = 2.0 / m_duzina;
    float sy = 2.0 / m_visina;

    glUseProgram(program);
    glBindVertexArray(vao);
    glDisable(GL_DEPTH_TEST);

    /* Enable blending, necessary for our alpha texture */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    GLfloat black[4] = { 0, 0, 0, 1 };
    //GLfloat red[4] = { 1, 0, 0, 1 };
    //GLfloat transparent_green[4] = { 0, 1, 0, 0.5 };

    /* Set color to black */
    glUniform4fv(uniform_color, 1, black);
    render_text("Nikola", a48, -1 + 10 * sx, 1 - 130 * sy, sx, sy);

    glBindVertexArray( 0 );
    glUseProgram(0);
    glEnable(GL_DEPTH_TEST);
}

void NkGlText::render_text(const char *text, atlas * a, float x, float y, float sx, float sy) {
    const uint8_t *p;

    /* Use the texture containing the atlas */
    glBindTexture(GL_TEXTURE_2D, a->tex);
    glUniform1i(uniform_tex, 0);

    /* Set up the VBO for our vertex data */
    glEnableVertexAttribArray(attribute_coord);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(attribute_coord, 4, GL_FLOAT, GL_FALSE, 0, 0);

    point coords[6 * strlen(text)];
    int c = 0;

    /* Loop through all characters */
    for (p = (const uint8_t *)text; *p; p++) {
        /* Calculate the vertex and texture coordinates */
        float x2 = x + a->c[*p].bl * sx;
        float y2 = -y - a->c[*p].bt * sy;
        float w = a->c[*p].bw * sx;
        float h = a->c[*p].bh * sy;

        /* Advance the cursor to the start of the next character */
        x += a->c[*p].ax * sx;
        y += a->c[*p].ay * sy;

        /* Skip glyphs that have no pixels */
        if (!w || !h){
            continue;
        }

        coords[c++] = (point){x2, -y2, a->c[*p].tx, a->c[*p].ty};
        coords[c++] = (point){x2 + w, -y2, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty};
        coords[c++] = (point) {x2, -y2 - h, a->c[*p].tx, a->c[*p].ty + a->c[*p].bh / a->h};
        coords[c++] = (point) {x2 + w, -y2, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty};
        coords[c++] = (point) {x2, -y2 - h, a->c[*p].tx, a->c[*p].ty + a->c[*p].bh / a->h};
        coords[c++] = (point) {x2 + w, -y2 - h, a->c[*p].tx + a->c[*p].bw / a->w, a->c[*p].ty + a->c[*p].bh / a->h};
    }

    /* Draw all the character on the screen in one go */

    glBufferData(GL_ARRAY_BUFFER, sizeof coords, coords, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, c);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisableVertexAttribArray(attribute_coord);

}

