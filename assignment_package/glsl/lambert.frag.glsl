#version 150
// ^ Change this to version 130 if you have compatibility issues

// This is a fragment shader. If you've opened this file first, please
// open and read lambert.vert.glsl before reading on.
// Unlike the vertex shader, the fragment shader actually does compute
// the shading of geometry. For every pixel in your program's output
// screen, the fragment shader is run for every bit of geometry that
// particular pixel overlaps. By implicitly interpolating the position
// data passed into the fragment shader by the vertex shader, the fragment shader
// can compute what color to apply to its pixel based on things like vertex
// position, light position, and vertex color.

uniform vec4 u_Color; // The color with which to render this instance of geometry.

uniform sampler2D u_Texture; // The texture to be read from by this shader


// These are the interpolated values out of the rasterizer, so you can't know
// their specific values without knowing the vertices that contributed to them
in vec4 fs_Pos;
in vec4 fs_Nor;

in vec3 fs_UV;

uniform int uTime;

out vec4 out_Col; // This is the final output color that you will see on your
                  // screen for the pixel that is currently being processed.


const vec3 sunColorDawn = vec3(255, 246, 79) / 255.0;
const vec3 sunColorDusk = vec3(255, 246, 79) / 255.0;
const vec3 sunColorDay = vec3(255, 249, 196) / 255.0;

const vec3 moonColor = vec3(200.0, 233.0, 248.0) / 255.0;


float random1(vec3 p) {
    return fract(sin(dot(p,vec3(127.1, 311.7, 191.999)))
                 *43758.5453);
}

//float mySmoothStep(float a, float b, float t) {
//    t = smoothstep(0, 1, t);
//    return mix(a, b, t);
//}

//float cubicTriMix(vec3 p) {
//    vec3 pFract = fract(p);
//    float llb = random1(floor(p) + vec3(0,0,0));
//    float lrb = random1(floor(p) + vec3(1,0,0));
//    float ulb = random1(floor(p) + vec3(0,1,0));
//    float urb = random1(floor(p) + vec3(1,1,0));

//    float llf = random1(floor(p) + vec3(0,0,1));
//    float lrf = random1(floor(p) + vec3(1,0,1));
//    float ulf = random1(floor(p) + vec3(0,1,1));
//    float urf = random1(floor(p) + vec3(1,1,1));

//    float mixLoBack = mySmoothStep(llb, lrb, pFract.x);
//    float mixHiBack = mySmoothStep(ulb, urb, pFract.x);
//    float mixLoFront = mySmoothStep(llf, lrf, pFract.x);
//    float mixHiFront = mySmoothStep(ulf, urf, pFract.x);

//    float mixLo = mySmoothStep(mixLoBack, mixLoFront, pFract.z);
//    float mixHi = mySmoothStep(mixHiBack, mixHiFront, pFract.z);

//    return mySmoothStep(mixLo, mixHi, pFract.y);
//}

//float fbm(vec3 p) {
//    float amp = 0.5;
//    float freq = 4.0;
//    float sum = 0.0;
//    for(int i = 0; i < 8; i++) {
//        sum += cubicTriMix(p * freq) * amp;
//        amp *= 0.5;
//        freq *= 2.0;
//    }
//    return sum;
//}

void main()
{
    int modVal = 1000;
    float time = mod(uTime, modVal) / modVal;
    float time2 = mod(uTime, (modVal*2)) / (modVal*2);
    float time4 = mod(uTime, (modVal*4)) / (modVal*4);
    vec3 sunMotion = vec3(0, time, -time);
    vec3 sunDirRise = normalize(vec3(0, 0, 1.f) + sunMotion);
    sunMotion = vec3(0, -time, -time);
    vec3 sunDirSet = normalize(vec3(0, 1, 0.f) + sunMotion);

    vec3 sunDir = sunDirRise;
    vec3 sunColor = vec3(255, 255, 190) / 255.0;

    if (time2 > 0.5) {
        sunDir = sunDirSet;
        sunColor = mix(sunColorDay, sunColorDusk, time);
    } else {
        sunColor = mix(sunColorDawn, sunColorDay, time);
    }
    //Night so we have a moon instead
    if (time4 >= 0.5) {
        vec3 moonDir = normalize(vec3(0.f, 1.f, 1.f));
        sunDir = moonDir;
        sunColor = 0.25 * moonColor;
    }
    vec4 fs_LightVec = vec4(sunDir, 0.f);  // Compute the direction in which the light source lies


    float t = mod(float(uTime), 17) / 17;
    t = clamp(t, 0.f, 1.f);

    vec2 uv = vec2(fs_UV);

    if (fs_UV.z == 1.f) {
        uv.x = fs_UV.x + t/64.f;
    }

    // Material base color (before shading)
    vec4 diffuseColor = texture(u_Texture, vec2(uv));

    if (fs_UV.z == 1.f && fs_UV.y <= 944.f/1024.f) {
        diffuseColor += vec4(0.f, 0.3f, 0.8f, 0.f);
    }

    float a = diffuseColor.w;
    //diffuseColor = diffuseColor * (0.5 * fbm(fs_Pos.xyz) + 0.5);

    // Calculate the diffuse term for Lambert shading
    float diffuseTerm = dot(normalize(fs_Nor), normalize(fs_LightVec));
    // Avoid negative lighting values
    diffuseTerm = clamp(diffuseTerm, 0, 1);
    
    float ambientTerm = 0.2;

    vec3 sunlight_contrib = diffuseTerm * sunColor;

    // Compute final shaded color
    vec3 final_col = min(sunlight_contrib + ambientTerm, vec3(1.f)) * diffuseColor.rgb;


    if (a == 0.f) {
        discard;
    }

    out_Col = vec4(final_col, a);
}
