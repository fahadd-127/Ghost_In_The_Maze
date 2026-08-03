#version 330 core

in vec3 vertex;
in vec3 normal;
in vec2 texUV;

in vec3 matamb;
in vec3 matdiff;
in vec3 matspec;
in float matshin;

uniform mat4 PM;
uniform mat4 VM;
uniform mat4 TG;
uniform mat3 NM;

// Dades per al fragment shader (Phong per fragment)
out vec2 fUV;
out vec3 fNormalSCO;
out vec3 fVertSCO;
out vec3 fVertMon;
out vec3 fMatAmb;
out vec3 fMatDiff;
out vec3 fMatSpec;
out float fMatShin;

void main()
{
    fUV = texUV;
    fMatAmb = matamb;
    fMatDiff = matdiff;
    fMatSpec = matspec;
    fMatShin = matshin;

    vec4 vertexSCO = VM * TG * vec4(vertex, 1.0);
    fNormalSCO = normalize(NM * normal);
    fVertSCO = vertexSCO.xyz;
    fVertMon = (TG * vec4(vertex, 1.0)).xyz;

    gl_Position = PM * vertexSCO;
}
