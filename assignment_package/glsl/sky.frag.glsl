#version 150

uniform mat4 u_ViewProj;    // We're actually passing the inverse of the viewproj
                            // from our CPU, but it's named u_ViewProj so we don't
                            // have to bother rewriting our ShaderProgram class

uniform ivec2 u_Dimensions; // Screen dimensions

uniform vec3 u_Eye; // Camera pos

uniform int uTime;

in vec4 fs_Pos;
out vec3 out_Col;

const float PI = 3.14159265359;
const float TWO_PI = 6.28318530718;

// Sunset palette
const vec3 sunset[5] = vec3[](vec3(255, 229, 119) / 255.0,
                               vec3(254, 192, 81) / 255.0,
                               vec3(255, 137, 103) / 255.0,
                               vec3(253, 96, 81) / 255.0,
                               vec3(57, 32, 51) / 255.0);
// Dusk palette
const vec3 dusk[5] = vec3[](vec3(144, 96, 144) / 255.0,
                            vec3(96, 72, 120) / 255.0,
                            vec3(72, 48, 120) / 255.0,
                            vec3(48, 24, 96) / 255.0,
                            vec3(0, 24, 72) / 255.0);

const vec3 nightSky = vec3(2.0/255.0, 1.0/255.0, 78.0/255.0);
const vec3 daySky = vec3(0.37f, 0.74f, 1.0f);

const vec3 sunColorDawn = vec3(255, 246, 79) / 255.0;
const vec3 sunColorDusk = vec3(255, 246, 79) / 255.0;
const vec3 sunColorDay = vec3(255, 249, 196) / 255.0;

const vec3 moonColor = vec3(200.0, 233.0, 248.0) / 255.0;

vec2 sphereToUV(vec3 p) {
    float phi = atan(p.z, p.x);
    if(phi < 0) {
        phi += TWO_PI;
    }
    float theta = acos(p.y);
    return vec2(1 - phi / TWO_PI, 1 - theta / PI);
}

vec3 uvToSunset(vec2 uv) {
    if(uv.y < 0.45) {
        return sunset[2];
    }
//    else if(uv.y < 0.4) {
//        return mix(sunset[0], sunset[1], (uv.y - 0.35) / 0.05);
//    }
//    else if(uv.y < 0.45) {
//        return mix(sunset[0], sunset[2], (uv.y - 0.4) / 0.05);
//    }
    else if(uv.y < 0.5) {
        return mix(sunset[2], sunset[3], (uv.y - 0.45) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(sunset[3], sunset[4], (uv.y - 0.5) / 0.1);
    }
    return sunset[4];
}

vec3 uvToDusk(vec2 uv) {
    if(uv.y < 0.35) {
        return dusk[0];
    }
    else if(uv.y < 0.4) {
        return mix(dusk[0], dusk[1], (uv.y - 0.35) / 0.05);
    }
    else if(uv.y < 0.45) {
        return mix(dusk[1], dusk[2], (uv.y - 0.4) / 0.05);
    }
    else if(uv.y < 0.5) {
        return mix(dusk[2], dusk[3], (uv.y - 0.45) / 0.05);
    }
    else if(uv.y < 0.6) {
        return mix(dusk[3], dusk[4], (uv.y - 0.5) / 0.1);
    }
    return dusk[4];
}

vec3 random3( vec3 p ) {
    return fract(sin(vec3(dot(p,vec3(127.1, 311.7, 191.999)),
                          dot(p,vec3(269.5, 183.3, 765.54)),
                          dot(p, vec3(420.69, 631.2,109.21))))
                 *43758.5453);
}

float WorleyNoise3D(vec3 p)
{
    // Tile the space
    vec3 pointInt = floor(p);
    vec3 pointFract = fract(p);

    float minDist = 1.0; // Minimum distance initialized to max.

    // Search all neighboring cells and this cell for their point
    for(int z = -1; z <= 1; z++)
    {
        for(int y = -1; y <= 1; y++)
        {
            for(int x = -1; x <= 1; x++)
            {
                vec3 neighbor = vec3(float(x), float(y), float(z));

                // Random point inside current neighboring cell
                vec3 point = random3(pointInt + neighbor);

                // Animate the point
                point = 0.5 + 0.5 * sin(uTime * 0.01 + 6.2831 * point); // 0 to 1 range

                // Compute the distance b/t the point and the fragment
                // Store the min dist thus far
                vec3 diff = neighbor + point - pointFract;
                float dist = length(diff);
                minDist = min(minDist, dist);
            }
        }
    }
    return minDist;
}


float worleyFBM(vec3 uv) {
    float sum = 0;
    float freq = 4;
    float amp = 0.5;
    for(int i = 0; i < 8; i++) {
        sum += WorleyNoise3D(uv * freq) * amp;
        freq *= 2;
        amp *= 0.5;
    }
    return sum;
}


void main()
{
    //times for day and night cycles
    int modVal = 20000;
    float time = mod(uTime, modVal) / modVal;
    float time2 = mod(uTime, (modVal*2)) / (modVal*2);
    float time4 = mod(uTime, (modVal*4)) / (modVal*4);

    //use the screen space coordinate from the vertex shader
    vec2 ndc = fs_Pos.xy;

    vec4 p = vec4(ndc.xy, 1, 1); // Pixel at the far clip plane
    p *= 1000.0; // Times far clip plane value
    p = /*Inverse of*/ u_ViewProj * p; // Convert from unhomogenized screen to world

    vec3 rayDir = normalize(p.xyz - u_Eye);

    vec2 uv = sphereToUV(rayDir);

    vec2 offset = vec2(0.0);

//    // Get a noise value in the range [-1, 1]
//    // by using Worley noise as the noise basis of FBM
    offset = vec2(worleyFBM(rayDir));
    offset *= 2.0;
    offset -= vec2(1.0);

    vec3 sunColor = sunColorDay;

    // Compute a gradient from the bottom of the sky-sphere to the top
    vec3 sunsetColor = uvToSunset(uv + offset * 0.1);
    vec3 duskColor = uvToDusk(uv + offset * 0.1);

    out_Col = sunsetColor;
    vec3 skyColor = daySky;
    if (time4 >= 0.5) {
        skyColor = nightSky;
    }

    // Add a glowing sun in the sky
    vec3 sunMotion = vec3(0, time, -time);
    vec3 sunDirRise = normalize(vec3(0, 0, 1.f) + sunMotion);
    sunMotion = vec3(0, -time, -time);
    vec3 sunDirSet = normalize(vec3(0, 1, 0.f) + sunMotion);

    vec3 sunDir = sunDirRise;
    if (time2 >= 0.5) {
        sunDir = sunDirSet;
        sunColor = mix(sunColorDay, sunColorDusk, time);
    } else {
        sunColor = mix(sunColorDawn, sunColorDay, time);
    }
    //Night so we have a moon instead
    if (time4 >= 0.5) {
        vec3 moonDir = normalize(vec3(0.f, 1.f, 1.f));
        sunDir = moonDir;
        sunColor = moonColor;
    } else {
        float raySunDot = dot(rayDir, sunDir);
        float SUNSET_THRESHOLD = 0.9;
        float DUSK_THRESHOLD = -0.1;
        if(raySunDot > SUNSET_THRESHOLD) {
            skyColor = sunsetColor;
        }
        // Any dot product between 0.6 and -0.1 is a LERP b/t sunset and dusk color
        else if(raySunDot > DUSK_THRESHOLD) {
            float t = (raySunDot - SUNSET_THRESHOLD) / (DUSK_THRESHOLD - SUNSET_THRESHOLD);
            skyColor = mix(out_Col, duskColor, t);
        }
        // Any dot product <= -0.1 are pure dusk color
        else {
            skyColor = duskColor;
        }
    }


    float sunSize = 10;
    float angle = acos(dot(rayDir, sunDir)) * 360.0 / PI;
    // If the angle between our ray dir and vector to center of sun
    // is less than the threshold, then we're looking at the sun
    if(angle < sunSize) {
        out_Col = sunColor;

        if (angle > 7.5) {
            out_Col = mix(sunColor, skyColor, (angle - 7.5)/2.5);
        }
    }
    // Otherwise our ray is looking into just the sky
    else {

        if (time4 >= 0.5) {
            out_Col = duskColor;
        } else {
            out_Col = skyColor;
        }
    }

}
