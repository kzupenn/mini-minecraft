#version 150
// ^ Change this to version 130 if you have compatibility issues

// Refer to the lambert shader files for useful comments

uniform sampler2D u_Texture; // The texture to be read from by this shader


in vec4 fs_Col;
in vec2 fs_UV;

out vec4 out_Col;

void main()
{
    vec4 foo = texture(u_Texture, vec2(fs_UV));
    foo[0] *= fs_Col[0];
    foo[1] *= fs_Col[1];
    foo[2] *= fs_Col[2];
    out_Col = foo;
}
