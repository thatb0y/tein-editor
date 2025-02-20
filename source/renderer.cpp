GLOBAL SDL_GLContext gl_context;
GLOBAL Window*    render_target;

GLOBAL quad renderer_viewport;
GLOBAL vec4 renderer_draw_color;

GLOBAL std::stack<quad> scissor_stack;

GLOBAL Shader untextured_shader;
GLOBAL Shader   textured_shader;
GLOBAL Shader       text_shader;

GLOBAL float texture_draw_scale_x;
GLOBAL float texture_draw_scale_y;

GLOBAL float font_draw_scale;

GLOBAL GLfloat max_gl_texture_size;

GLOBAL Buffer_Mode immediate_buffer_draw_mode;

GLOBAL Vertex_Buffer draw_buffer;
GLOBAL Vertex_Buffer tile_buffer;
GLOBAL Vertex_Buffer text_buffer;

// Batched tile rendering.
GLOBAL vec4     tile_draw_color;
GLOBAL Texture* tile_texture;
// Batched text rendering.
GLOBAL vec4     text_draw_color;
GLOBAL Font*    text_font;

FILDEF quad internal__convert_viewport (quad viewport)
{
    // Converts a viewport in our top-left format to GL's bottom-left format.
    quad converted;

    converted.x = viewport.x;
    converted.y = (get_render_target_h() - (viewport.y + viewport.h));
    converted.w = viewport.w;
    converted.h = viewport.h;

    return converted;
}

FILDEF void internal__set_texture0_uniform (Shader shader, GLenum unit)
{
    GLint location = glGetUniformLocation(shader, "texture0");
    glUniform1i(location, unit);
}

FILDEF GLenum internal__convert_matrix_mode_to_glenum (Matrix_Mode mode)
{
    switch (mode)
    {
        case (Matrix_Mode::PROJECTION): return GL_PROJECTION; break;
        case (Matrix_Mode::MODELVIEW): return GL_MODELVIEW; break;
    }
    ASSERT(false);
    return 0;
}

FILDEF bool init_renderer ()
{
    gl_context = SDL_GL_CreateContext(get_window("Main").window);
    if (!gl_context)
    {
        LOG_ERROR(ERR_MIN, "Failed to create GL context! (%s)", SDL_GetError());
        return false;
    }

    if (!gladLoadGLLoader(SDL_GL_GetProcAddress))
    {
        LOG_ERROR(ERR_MIN, "Failed to load OpenGL procedures!");
        return false;
    }

    LOG_DEBUG("OpenGL Backend Information:");
    LOG_DEBUG("GL Version: %s", glGetString(GL_VERSION));
    LOG_DEBUG("GLSL Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));
    LOG_DEBUG("Renderer: %s", glGetString(GL_RENDERER));
    LOG_DEBUG("Vendor: %s", glGetString(GL_VENDOR));

    renderer_draw_color  = vec4(1,1,1,1);
    texture_draw_scale_x = 1;
    texture_draw_scale_y = 1;
    font_draw_scale      = 1;

    glGetFloatv(GL_MAX_TEXTURE_SIZE, &max_gl_texture_size);

    untextured_shader = load_shader_resource("shaders/300/untextured.shader");
    if (!untextured_shader)
    {
        LOG_ERROR(ERR_MAX, "Failed to load the untextured shader!");
        return false;
    }

    textured_shader = load_shader_resource("shaders/300/textured.shader");
    if (!textured_shader)
    {
        LOG_ERROR(ERR_MAX, "Failed to load the textured shader!");
        return false;
    }

    // We carry on even on failure because text will still be drawn it
    // will just be extremely ugly... but it's not worth failing over.
    text_shader = load_shader_resource("shaders/300/text.shader");
    if (!text_shader)
    {
        LOG_ERROR(ERR_MED, "Failed to load the text shader!");
    }

    // By default we render to the main window.
    set_render_target(&get_window("Main"));

    set_viewport(0, 0, get_render_target_w(), get_render_target_h());

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);

    create_vertex_buffer(draw_buffer);
    create_vertex_buffer(tile_buffer);
    create_vertex_buffer(text_buffer);

    immediate_buffer_draw_mode = Buffer_Mode::TRIANGLE_STRIP;

    return true;
}

FILDEF void quit_renderer ()
{
    free_vertex_buffer(draw_buffer);
    free_vertex_buffer(tile_buffer);
    free_vertex_buffer(text_buffer);

    free_shader(untextured_shader);
    free_shader(  textured_shader);
    free_shader(      text_shader);

    SDL_GL_DeleteContext(gl_context);
    gl_context = NULL;
}

FILDEF void render_clear (vec4 clear)
{
    glClearColor(clear.r, clear.g, clear.b, clear.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

FILDEF void render_present ()
{
    ASSERT(render_target);
    if (render_target)
    {
        SDL_GL_SwapWindow(render_target->window);
    }
}

STDDEF vec2 screen_to_world (vec2 screen)
{
    // GL expects bottom-left so we have to convert our viewport first.
    quad v = internal__convert_viewport(renderer_viewport);

    // We also need to do flip the Y coordinate to use this system.
    screen.y = get_render_target_h() - screen.y;
    vec3 coord(screen.x, screen.y, 0);

    mat4 projection;
    mat4 modelview;

    float matrix[16];

    glGetFloatv(GL_PROJECTION_MATRIX, matrix);
    projection = glm::make_mat4(matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    modelview = glm::make_mat4(matrix);

    vec4 viewport(v.x, v.y, v.w, v.h);

    coord = glm::unProject(coord, modelview, projection, viewport);

    vec2 world(coord.x, coord.y);

    return world;
}

STDDEF vec2 world_to_screen (vec2 world)
{
    // GL expects bottom-left so we have to convert our viewport first.
    quad v = internal__convert_viewport(renderer_viewport);

    vec3 coord(world.x, world.y, 0);

    mat4 projection;
    mat4 modelview;

    float matrix[16];

    glGetFloatv(GL_PROJECTION_MATRIX, matrix);
    projection = glm::make_mat4(matrix);
    glGetFloatv(GL_MODELVIEW_MATRIX, matrix);
    modelview = glm::make_mat4(matrix);

    vec4 viewport(v.x, v.y, v.w, v.h);

    coord = glm::project(coord, modelview, projection, viewport);

    // We also need to do flip the Y coordinate to use this system.
    vec2 screen(coord.x, coord.y);

    screen.x -= v.x;
    screen.y = get_render_target_h() - (screen.y + renderer_viewport.y);

    return screen;
}

FILDEF float get_max_texture_size ()
{
    return CAST(float, max_gl_texture_size);
}

FILDEF Window* get_render_target ()
{
    return render_target;
}

FILDEF void set_render_target (Window* window)
{
    render_target = window;
    ASSERT(render_target);

    if (SDL_GL_MakeCurrent(render_target->window, gl_context) < 0)
    {
        render_target = NULL;
        LOG_ERROR(ERR_MED, "Failed to set render target! (%s)", SDL_GetError());
    }
}

FILDEF float get_render_target_w ()
{
    int w = 0;
    if (render_target) SDL_GL_GetDrawableSize(render_target->window, &w, NULL);
    return CAST(float, w);
}

FILDEF float get_render_target_h ()
{
    int h = 0;
    if (render_target) SDL_GL_GetDrawableSize(render_target->window, NULL, &h);
    return CAST(float, h);
}

STDDEF void set_orthographic (float l, float r, float b, float t)
{
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(l, r, b, t, 0, 1);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

STDDEF void set_viewport (float x, float y, float w, float h)
{
    // GL expects bottom-left so we have to flip the Y coordinate around.
    renderer_viewport = { x, y, w, h };
    quad v = internal__convert_viewport(renderer_viewport);

    GLint   vx = CAST(GLint,   v.x);
    GLint   vy = CAST(GLint,   v.y);
    GLsizei vw = CAST(GLsizei, v.w);
    GLsizei vh = CAST(GLsizei, v.h);

    glViewport(vx, vy, vw, vh);

    set_orthographic(0, w, h, 0);
}

FILDEF void set_viewport (quad v)
{
    set_viewport(v.x, v.y, v.w, v.h);
}

FILDEF quad get_viewport ()
{
    return renderer_viewport;
}

FILDEF void set_draw_color (float r, float g, float b, float a)
{
    renderer_draw_color = { r, g, b, a };
}

FILDEF void set_draw_color (vec4 color)
{
    renderer_draw_color = color;
}

FILDEF void set_line_width (float width)
{
    glLineWidth(width);
}

FILDEF void set_texture_draw_scale (float sx, float sy)
{
    texture_draw_scale_x = sx;
    texture_draw_scale_y = sy;
}

FILDEF float get_texture_draw_scale_x ()
{
    return texture_draw_scale_x;
}

FILDEF float get_texture_draw_scale_y ()
{
    return texture_draw_scale_y;
}

FILDEF void set_font_draw_scale (float scale)
{
    font_draw_scale = scale;
}

FILDEF float get_font_draw_scale ()
{
    return font_draw_scale;
}

STDDEF void begin_scissor (float x, float y, float w, float h)
{
    // Our version of setting the scissor region takes the currently set
    // viewport into consideration rather than basing the region on the
    // size of the screen/window. As a result, we need to perform a few
    // operations on the values passed in to make sure this the case.

    // We push scissor regions onto a stack so we can stack scissor calls.
    // This is particularly useful for the GUI which uses many scissors.
    if (scissor_stack.size() == 0) glEnable(GL_SCISSOR_TEST);
    scissor_stack.push({ x, y, w, h });

    // GL expects bottom-left so we have to flip the Y coordinate around.
    GLint   sx = CAST(GLint,   renderer_viewport.x + x);
    GLint   sy = CAST(GLint,   get_render_target_h() - (renderer_viewport.y + (y + h)));
    GLsizei sw = CAST(GLsizei, w);
    GLsizei sh = CAST(GLsizei, h);

    glScissor(sx, sy, sw, sh);
}

STDDEF void end_scissor ()
{
    // Pop the last scissor region off the stack.
    quad s = scissor_stack.top();
    scissor_stack.pop();

    // GL expects bottom-left so we have to flip the Y coordinate around.
    GLint   x = CAST(GLint,   renderer_viewport.x + s.x);
    GLint   y = CAST(GLint,   get_render_target_h() - (renderer_viewport.y + (s.y + s.h)));
    GLsizei w = CAST(GLsizei, s.w);
    GLsizei h = CAST(GLsizei, s.h);

    glScissor(x, y, w, h);

    if (scissor_stack.size() == 0) glDisable(GL_SCISSOR_TEST);
}

FILDEF void begin_stencil ()
{
    glEnable(GL_STENCIL_TEST);

    glClearStencil(0);
    glClear(GL_STENCIL_BUFFER_BIT);
}

FILDEF void end_stencil ()
{
    stencil_mode_draw();
    glDisable(GL_STENCIL_TEST);
}

FILDEF void stencil_mode_erase ()
{
    glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
    glDepthMask(GL_FALSE);
    glStencilFunc(GL_ALWAYS, CAST(GLuint, 1), CAST(GLuint, ~0));
    glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
}

FILDEF void stencil_mode_draw ()
{
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
    glDepthMask(GL_TRUE);
    glStencilFunc(GL_NOTEQUAL, CAST(GLuint, 1), CAST(GLuint, ~0));
    glStencilOp(GL_KEEP, GL_KEEP, GL_KEEP);
}

FILDEF void draw_line (float x1, float y1, float x2, float y2)
{
    glUseProgram(untextured_shader);

    put_buffer_vertex(draw_buffer, { vec2(x1,y1), vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { vec2(x2,y2), vec2(0,0), renderer_draw_color });

    draw_vertex_buffer(draw_buffer, Buffer_Mode::LINES);
    clear_vertex_buffer(draw_buffer);
}

FILDEF void draw_quad (float x1, float y1, float x2, float y2)
{
    glUseProgram(untextured_shader);

    // We want to use .5f otherwise the lines don't draw where we want them.
    x1 += .5f;
    y1 += .5f;
    x2 -= .5f;
    y2 -= .5f;

    put_buffer_vertex(draw_buffer, { vec2(x1,y1), vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { vec2(x2,y1), vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { vec2(x2,y2), vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { vec2(x1,y2), vec2(0,0), renderer_draw_color });

    draw_vertex_buffer(draw_buffer, Buffer_Mode::LINE_LOOP);
    clear_vertex_buffer(draw_buffer);
}

FILDEF void fill_quad (float x1, float y1, float x2, float y2)
{
    glUseProgram(untextured_shader);

    put_buffer_vertex(draw_buffer, { vec2(x1,y2), vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { vec2(x1,y1), vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { vec2(x2,y2), vec2(0,0), renderer_draw_color });
    put_buffer_vertex(draw_buffer, { vec2(x2,y1), vec2(0,0), renderer_draw_color });

    draw_vertex_buffer(draw_buffer, Buffer_Mode::TRIANGLE_STRIP);
    clear_vertex_buffer(draw_buffer);
}

STDDEF void draw_texture (const Texture& tex, float x, float y, const quad* clip)
{
    glBindTexture(GL_TEXTURE_2D, tex.handle);
    glEnable(GL_TEXTURE_2D);

    defer { glDisable(GL_TEXTURE_2D); };

    glUseProgram(textured_shader);
    internal__set_texture0_uniform(textured_shader, GL_TEXTURE0);

    float cx1, cy1, cx2, cy2;
    float w, h;

    if (clip)
    {
        cx1 =       (clip->x / tex.w);
        cy1 =       (clip->y / tex.h);
        cx2 = cx1 + (clip->w / tex.w);
        cy2 = cy1 + (clip->h / tex.h);

        w = clip->w * texture_draw_scale_x;
        h = clip->h * texture_draw_scale_y;
    }
    else
    {
        cx1 = 0;
        cy1 = 0;
        cx2 = 1;
        cy2 = 1;

        w = tex.w * texture_draw_scale_x;
        h = tex.h * texture_draw_scale_y;
    }

    float x1 = x  - (w / 2); // Center anchor.
    float y1 = y  - (h / 2); // Center anchor.
    float x2 = x1 + w;
    float y2 = y1 + h;

    put_buffer_vertex(draw_buffer, { vec2(x1,y2), vec2(cx1,cy2), tex.color });
    put_buffer_vertex(draw_buffer, { vec2(x1,y1), vec2(cx1,cy1), tex.color });
    put_buffer_vertex(draw_buffer, { vec2(x2,y2), vec2(cx2,cy2), tex.color });
    put_buffer_vertex(draw_buffer, { vec2(x2,y1), vec2(cx2,cy1), tex.color });

    draw_vertex_buffer(draw_buffer, Buffer_Mode::TRIANGLE_STRIP);
    clear_vertex_buffer(draw_buffer);
}

STDDEF void draw_text (const Font& fnt, float x, float y, std::string text)
{
    glBindTexture(GL_TEXTURE_2D, fnt.cache.at(fnt.current_pt_size).handle);
    glEnable(GL_TEXTURE_2D);

    defer { glDisable(GL_TEXTURE_2D); };

    glUseProgram(text_shader);
    internal__set_texture0_uniform(text_shader, GL_TEXTURE0);

    std::vector<Vertex> verts;

    int index      = 0;
    int prev_index = 0;

    float cx = x;
    float cy = y;

    float scale = font_draw_scale;

    const Texture& cache = fnt.cache.at(fnt.current_pt_size);
    auto& glyphs = fnt.glyphs.at(fnt.current_pt_size);

    for (const char* c=text.c_str(); *c; ++c)
    {
        if (*c < 0 || *c >= TOTAL_GLYPH_COUNT) continue;

        cx += (get_font_kerning(fnt, *c, index, prev_index) * scale);

        switch (*c)
        {
            case ('\r'): cx = x;                                                       break;
            case ('\n'): cx = x, cy += (fnt.line_gap.at(fnt.current_pt_size) * scale); break;
            case ('\t'): cx += get_font_tab_width(fnt) * scale;                        break;

            default:
            {
                const Font_Glyph& glyph = glyphs.at(*c);
                const quad& clip = glyph.bounds;

                float bearing_x = glyph.bearing.x * scale;
                float bearing_y = glyph.bearing.y * scale;

                float advance = glyph.advance * scale;

                float cx1 =       (clip.x / cache.w);
                float cy1 =       (clip.y / cache.h);
                float cx2 = cx1 + (clip.w / cache.w);
                float cy2 = cy1 + (clip.h / cache.h);

                float w = clip.w * scale;
                float h = clip.h * scale;

                float x1 = roundf(cx + bearing_x);
                float y1 = roundf(cy - bearing_y);
                float x2 = roundf(x1 + w);
                float y2 = roundf(y1 + h);

                put_buffer_vertex(draw_buffer, { vec2(x1,y2), vec2(cx1,cy2), fnt.color }); // V0
                put_buffer_vertex(draw_buffer, { vec2(x1,y1), vec2(cx1,cy1), fnt.color }); // V1
                put_buffer_vertex(draw_buffer, { vec2(x2,y2), vec2(cx2,cy2), fnt.color }); // V2
                put_buffer_vertex(draw_buffer, { vec2(x2,y2), vec2(cx2,cy2), fnt.color }); // V2
                put_buffer_vertex(draw_buffer, { vec2(x1,y1), vec2(cx1,cy1), fnt.color }); // V1
                put_buffer_vertex(draw_buffer, { vec2(x2,y1), vec2(cx2,cy1), fnt.color }); // V3

                cx += advance;
            } break;
        }
    }

    draw_vertex_buffer(draw_buffer, Buffer_Mode::TRIANGLES);
    clear_vertex_buffer(draw_buffer);
}

FILDEF void begin_draw (Buffer_Mode mode)
{
    immediate_buffer_draw_mode = mode;
}

FILDEF void end_draw ()
{
    glUseProgram(untextured_shader);

    draw_vertex_buffer(draw_buffer, immediate_buffer_draw_mode);
    clear_vertex_buffer(draw_buffer);
}

FILDEF void put_vertex (float x, float y, vec4 color)
{
    put_buffer_vertex(draw_buffer, { vec2(x,y), vec2(0,0), color });
}

FILDEF void push_matrix (Matrix_Mode mode)
{
    glMatrixMode(internal__convert_matrix_mode_to_glenum(mode));
    glPushMatrix();
    glLoadIdentity();
}

FILDEF void pop_matrix (Matrix_Mode mode)
{
    glMatrixMode(internal__convert_matrix_mode_to_glenum(mode));
    glPopMatrix();
}

FILDEF void translate (float x, float y)
{
    glTranslatef(x, y, 0);
}

FILDEF void rotate (float x, float y, float angle)
{
    glRotatef(angle, x, y, 0);
}

FILDEF void scale (float x, float y)
{
    glScalef(x, y, 1);
}

FILDEF void set_tile_batch_texture (Texture& tex)
{
    tile_texture = &tex;
}

FILDEF void set_text_batch_font (Font& fnt)
{
    text_font = &fnt;
}

FILDEF void set_tile_batch_color (vec4 color)
{
    tile_draw_color = color;
}

FILDEF void set_text_batch_color (vec4 color)
{
    text_draw_color = color;
}

FILDEF void draw_batched_tile (float x, float y, const quad* clip)
{
    ASSERT(tile_texture);

    float cx1 =       (clip->x / tile_texture->w);
    float cy1 =       (clip->y / tile_texture->h);
    float cx2 = cx1 + (clip->w / tile_texture->w);
    float cy2 = cy1 + (clip->h / tile_texture->h);

    float w = clip->w * texture_draw_scale_x;
    float h = clip->h * texture_draw_scale_y;

    float x1 = x  - (w / 2); // Center anchor.
    float y1 = y  - (h / 2); // Center anchor.
    float x2 = x1 + w;
    float y2 = y1 + h;

    put_buffer_vertex(tile_buffer, { vec2(x1,y2), vec2(cx1,cy2), tile_draw_color }); // V0
    put_buffer_vertex(tile_buffer, { vec2(x1,y1), vec2(cx1,cy1), tile_draw_color }); // V1
    put_buffer_vertex(tile_buffer, { vec2(x2,y2), vec2(cx2,cy2), tile_draw_color }); // V2
    put_buffer_vertex(tile_buffer, { vec2(x2,y2), vec2(cx2,cy2), tile_draw_color }); // V2
    put_buffer_vertex(tile_buffer, { vec2(x1,y1), vec2(cx1,cy1), tile_draw_color }); // V1
    put_buffer_vertex(tile_buffer, { vec2(x2,y1), vec2(cx2,cy1), tile_draw_color }); // V3
}

FILDEF void draw_batched_text (float x, float y, std::string text)
{
    int index      = 0;
    int prev_index = 0;

    float cx = x;
    float cy = y;

    const Font& fnt = *text_font;
    float scale = font_draw_scale;

    const auto& cache    = fnt.cache   .at(fnt.current_pt_size);
    const auto& glyphs   = fnt.glyphs  .at(fnt.current_pt_size);
    const auto& line_gap = fnt.line_gap.at(fnt.current_pt_size);

    for (const char* c=text.c_str(); *c; ++c)
    {
        if (*c < 0 || *c >= TOTAL_GLYPH_COUNT) continue;

        cx += (get_font_kerning(fnt, *c, index, prev_index) * scale);

        switch (*c)
        {
            case ('\r'): cx = x;                                  break;
            case ('\n'): cx = x, cy += (line_gap * scale);        break;
            case ('\t'): cx += (get_font_tab_width(fnt) * scale); break;

            default:
            {
                const Font_Glyph& glyph = glyphs.at(*c);
                const quad& clip = glyph.bounds;

                float bearing_x = glyph.bearing.x * scale;
                float bearing_y = glyph.bearing.y * scale;

                float advance = glyph.advance * scale;

                float cx1 =       (clip.x / cache.w);
                float cy1 =       (clip.y / cache.h);
                float cx2 = cx1 + (clip.w / cache.w);
                float cy2 = cy1 + (clip.h / cache.h);

                float w = clip.w * scale;
                float h = clip.h * scale;

                float x1 = roundf(cx + bearing_x);
                float y1 = roundf(cy - bearing_y);
                float x2 = roundf(x1 + w);
                float y2 = roundf(y1 + h);

                put_buffer_vertex(text_buffer, { vec2(x1,y2), vec2(cx1,cy2), text_draw_color }); // V0
                put_buffer_vertex(text_buffer, { vec2(x1,y1), vec2(cx1,cy1), text_draw_color }); // V1
                put_buffer_vertex(text_buffer, { vec2(x2,y2), vec2(cx2,cy2), text_draw_color }); // V2
                put_buffer_vertex(text_buffer, { vec2(x2,y2), vec2(cx2,cy2), text_draw_color }); // V2
                put_buffer_vertex(text_buffer, { vec2(x1,y1), vec2(cx1,cy1), text_draw_color }); // V1
                put_buffer_vertex(text_buffer, { vec2(x2,y1), vec2(cx2,cy1), text_draw_color }); // V3

                cx += advance;
            } break;
        }
    }
}

FILDEF void flush_batched_tile ()
{
    glBindTexture(GL_TEXTURE_2D, tile_texture->handle);
    glEnable(GL_TEXTURE_2D);

    glUseProgram(textured_shader);

    draw_vertex_buffer(tile_buffer, Buffer_Mode::TRIANGLES);
    clear_vertex_buffer(tile_buffer);

    glDisable(GL_TEXTURE_2D);
}

FILDEF void flush_batched_text ()
{
    glBindTexture(GL_TEXTURE_2D, text_font->cache.at(text_font->current_pt_size).handle);
    glEnable(GL_TEXTURE_2D);

    glUseProgram(text_shader);

    draw_vertex_buffer(text_buffer, Buffer_Mode::TRIANGLES);
    clear_vertex_buffer(text_buffer);

    glDisable(GL_TEXTURE_2D);
}

FILDEF void create_vertex_buffer (Vertex_Buffer& buffer)
{
    glGenVertexArrays(1, &buffer.vao);
    glBindVertexArray(buffer.vao);

    glGenBuffers(1, &buffer.vbo);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

    glVertexPointer  (2, GL_FLOAT, sizeof(Vertex), (void*)(0*sizeof(float)));
    glTexCoordPointer(2, GL_FLOAT, sizeof(Vertex), (void*)(2*sizeof(float)));
    glColorPointer   (4, GL_FLOAT, sizeof(Vertex), (void*)(4*sizeof(float)));

    glEnableClientState(GL_VERTEX_ARRAY       );
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glEnableClientState(GL_COLOR_ARRAY        );
}

FILDEF void free_vertex_buffer (Vertex_Buffer& buffer)
{
    if (glDeleteVertexArrays) glDeleteVertexArrays(1, &buffer.vao);
    if (glDeleteBuffers) glDeleteBuffers(1, &buffer.vbo);

    buffer.verts.clear();
}

FILDEF void put_buffer_vertex (Vertex_Buffer& buffer, Vertex v)
{
    buffer.verts.push_back(v);
}

FILDEF void draw_vertex_buffer (Vertex_Buffer& buffer, Buffer_Mode mode)
{
    if (buffer.verts.empty()) return;

    glBindVertexArray(buffer.vao);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.vbo);

    GLsizeiptr size = buffer.verts.size() * sizeof(Vertex);
    glBufferData(GL_ARRAY_BUFFER, size, &buffer.verts[0], GL_DYNAMIC_DRAW);

    glDrawArrays(CAST(GLenum, mode), 0, CAST(GLsizei, buffer.verts.size()));
}

FILDEF void clear_vertex_buffer (Vertex_Buffer& buffer)
{
    buffer.verts.clear();
}
