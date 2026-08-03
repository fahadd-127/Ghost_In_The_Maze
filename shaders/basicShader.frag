#version 330 core

uniform int textActive;
uniform sampler2D text;

uniform mat4 VM;
uniform vec3 llumAmbientGlobal;

uniform int solActiu;
uniform vec3 colorFocus;
uniform vec3 posFocus;

uniform int llanternaActiva;
uniform vec3 colorLlanterna;
uniform vec3 posLlanterna;

uniform int llumFantasmaActiva;
uniform vec3 colorLlumFantasma;
uniform vec3 posLlumFantasma;

#define MAX_MONEDES_LLUM 10
uniform int numMonedesLlum;
uniform vec3 posMonedaLlum[MAX_MONEDES_LLUM];
uniform vec3 dirMonedaLlum[MAX_MONEDES_LLUM];
uniform vec3 colorMonedaLlum;

in vec2 fUV;
in vec3 fNormalSCO;
in vec3 fVertSCO;
in vec3 fVertMon;
in vec3 fMatAmb;
in vec3 fMatDiff;
in vec3 fMatSpec;
in float fMatShin;

out vec4 FragColor;

vec3 diffuseComponent(vec3 N, vec3 L, vec3 colLlum, vec3 matDiff)
{
    float ndotl = dot(N, L);
    if (ndotl <= 0.0)
        return vec3(0.0);
    return colLlum * matDiff * ndotl;
}

vec3 specularComponent(vec3 N, vec3 L, vec3 V, vec3 colLlum, vec3 matSpec)
{
    if (dot(N, L) < 0.0 || fMatShin <= 0.0)
        return vec3(0.0);

    vec3 R = reflect(-L, N);
    float rdotv = dot(R, V);
    if (rdotv < 0.0)
        return vec3(0.0);

    float shine = pow(rdotv, max(fMatShin, 1.0));
    return matSpec * colLlum * shine;
}

vec3 contribucioFocusDireccional(vec3 dirSolMon, vec3 colFocus, vec3 matDiff, vec3 matSpec)
{
    vec3 N = normalize(fNormalSCO);
    vec3 V = normalize(-fVertSCO);
    vec3 L = normalize((VM * vec4(dirSolMon, 0.0)).xyz);

    return diffuseComponent(N, L, colFocus, matDiff)
         + specularComponent(N, L, V, colFocus, matSpec);
}

vec3 contribucioFocusPunt(vec3 posFocusMon, vec3 colFocus, vec3 matDiff, vec3 matSpec)
{
    vec3 N = normalize(fNormalSCO);
    vec3 V = normalize(-fVertSCO);
    vec3 posFocusSCO = (VM * vec4(posFocusMon, 1.0)).xyz;
    vec3 L = normalize(posFocusSCO - fVertSCO);

    return diffuseComponent(N, L, colFocus, matDiff)
         + specularComponent(N, L, V, colFocus, matSpec);
}

float atenuacioDistanciaMoneda(float dist)
{
    const float abast = 4.0;
    const float k = 6.0;
    return 1.0 / (1.0 + exp(k * (dist - abast)));
}

vec3 contribucioFocusMoneda(vec3 posMon, vec3 dirMon, vec3 colFocus, vec3 matDiff, vec3 matSpec)
{
    float dist = length(posMon - fVertMon);
    if (dist > 4.0)
        return vec3(0.0);

    float attenDist = atenuacioDistanciaMoneda(dist);

    vec3 N = normalize(fNormalSCO);
    vec3 V = normalize(-fVertSCO);
    vec3 posFocusSCO = (VM * vec4(posMon, 1.0)).xyz;
    vec3 L = normalize(posFocusSCO - fVertSCO);
    vec3 d = normalize((VM * vec4(dirMon, 0.0)).xyz);

    float spot = clamp(dot(d, L), 0.0, 1.0);
    float factor = spot * attenDist;

    vec3 diff = diffuseComponent(N, L, colFocus, matDiff);
    vec3 spec = specularComponent(N, L, V, colFocus, matSpec);
    return factor * (diff + spec);
}

vec3 calcularIluminacio(bool ambTextura)
{
    vec3 matDiff = ambTextura ? vec3(1.0) : fMatDiff;
    vec3 matAmb = fMatAmb;
    vec3 matSpec = ambTextura ? vec3(0.25) : fMatSpec;

    vec3 color = llumAmbientGlobal * matAmb;

    if (solActiu == 1)
        color += contribucioFocusDireccional(posFocus, colorFocus, matDiff, matSpec);
    if (llanternaActiva == 1)
        color += contribucioFocusPunt(posLlanterna, colorLlanterna, matDiff, matSpec);
    if (llumFantasmaActiva == 1)
        color += contribucioFocusPunt(posLlumFantasma, colorLlumFantasma, matDiff, matSpec);

    for (int i = 0; i < numMonedesLlum; ++i) {
        color += contribucioFocusMoneda(posMonedaLlum[i], dirMonedaLlum[i], colorMonedaLlum,
                                        matDiff, matSpec);
    }

    return color;
}

void main()
{
    vec3 colorFinal;

    if (textActive == 1) {
        vec3 texColor = texture(text, fUV).rgb;
        vec3 ilum = calcularIluminacio(true);
        colorFinal = texColor * ilum;
    } else {
        colorFinal = calcularIluminacio(false);
    }

    FragColor = vec4(colorFinal, 1.0);
}
