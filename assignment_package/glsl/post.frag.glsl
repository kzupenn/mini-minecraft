#version 150

in vec2 fs_UV;

out vec4 out_Col;

uniform sampler2D u_Texture;
uniform int u_Type;

void main()
{
    // TODO Homework 5
    vec4 color = texture(u_Texture, fs_UV);
    if (u_Type == 0 || u_Type == 3) out_Col = color;
    else {
        vec4 mixer;
        if (u_Type == 1) mixer = vec4(0, 0, 1, 0.5);
        else if (u_Type == 2) mixer = vec4(0.8, 0.2, 0, 0.5);
        out_Col = mix(color, mixer, 0.3);
    }
}
