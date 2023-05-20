#version 330 core
out vec4 FragColor;

#define FAR_DISTANCE 1000000.0
#define SPHERE_COUNT 6
#define BOX_COUNT 2
#define MAX_DEPTH 8
#define PI 3.1415926535

Sphere spheres[SPHERE_COUNT];
Box boxes[BOX_COUNT];

struct Material {
    sampler2D diffuse;
    sampler2D specular;
    float shininess;
    vec3 color;

    vec3 emmitance;
    vec3 reflectance;
    float roughness;
    float opacity;
}; 

struct Box 
{
    Material material;
    vec3 halfSize;
    mat3 rotation;
    vec3 position;
};

struct Sphere
{
    Material material;
    vec3 position;
    float radius;
};

struct DirLight {
    vec3 direction;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct PointLight {
    vec3 position;
    
    float constant;
    float linear;
    float quadratic;
	
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SpotLight {
    vec3 position;
    vec3 direction;
    float cutOff;
    float outerCutOff;
  
    float constant;
    float linear;
    float quadratic;
  
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;       
};

#define NR_POINT_LIGHTS 8

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoords;

uniform vec3 viewPos;
uniform DirLight dirLight;
uniform PointLight pointLights[NR_POINT_LIGHTS];
uniform SpotLight spotLight;
uniform Material material;

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir);
vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir);
bool IntersectRaySphere(vec3 origin, vec3 direction, Sphere sphere, out float fraction, out vec3 normal);
bool IntersectRayBox(vec3 origin, vec3 direction, Box box, out float fraction, out vec3 normal);
bool CastRay(vec3 rayOrigin, vec3 rayDirection, out float fraction, out vec3 normal, out Material material);
vec3 TracePath(vec3 rayOrigin, vec3 rayDirection);
void RandomHemispherePoint(vec2 rand);
vec3 NormalOrientedHemispherePoint(vec2 rand, vec3 n);
float RandomNoise(vec2 co);
float FresnelSchlick(float nIn, float nOut, vec3 direction, vec3 normal);

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = CalcDirLight(dirLight, norm, viewDir);

    for(int i = 0; i < NR_POINT_LIGHTS; i++)
        result += CalcPointLight(pointLights[i], norm, FragPos, viewDir);   

    result += CalcSpotLight(spotLight, norm, FragPos, viewDir);    
    
    FragColor = vec4(result, 1.0) * vec4(material.color, 1.0f);
}

bool IntersectRaySphere(vec3 origin, vec3 direction, Sphere sphere, out float fraction, out vec3 normal)
{
    vec3 L = origin - sphere.position;
    float a = dot(direction, direction);
    float b = 2.0 * dot(L, direction);
    float c = dot(L, L) - sphere.radius * sphere.radius;
    float D = b * b - 4 * a * c;
    if(D < 0.0)
        return false;
    float r1 = (-b - sqrt(D)) / (2.0 * a);
    float r1 = (-b + sqrt(D)) / (2.0 * a);
    if(r1 > 0.0)
        fraction = r1;
    else if (r2 > 0.0)
        fraction = r2;
    else
        return false;
    normal = normalize(direction * fraction + L);
    return true;
}

bool IntersectRayBox(vec3 origin, vec3 direction, Box box, out float fraction, out vec3 normal)
{
    vec3 rd = box.rotation * direction;
    vec3 ro = box.rotation * (origin - box.position);
    vec3 m = vec3(1.0) / rd; 
    vec3 s = vec3((rd.x < 0.0) ? 1.0 : -1.0,
        (rd.y < 0.0) ? 1.0 : -1.0,
        (rd.z < 0.0) ? 1.0 : -1.0);
    vec3 t1 = m * (-ro + s * box.halfSize);
    vec3 t2 = m * (-ro - s * box.halfSize);
    float tN = max(max(t1.x, t1.y), t1.z);
    float tF = max(max(t2.x, t2.y), t2.z);
    if(tN > tF || tF < 0.0)
        return false;
    mat3 txi = transpose(box.rotation);
    if(t1.x > t1.y && tF < 0.0)
        normal = txi[0] * s.x;
    else if(t1.y > t1.z)
        normal = txi[1] * s.y;
    else
        normal = txi[2] * s.z;
    fraction = tN;
    return true;
}

bool CastRay(vec3 rayOrigin, vec3 rayDirection, out float fraction, out vec3 normal, out Material material)
{
    float minDistance = FAR_DISTANCE;
    for (int i = 0; i < SPHERE_COUNT; i++)
    {
        float D;
        vec3 N;
        if (IntersectRaySphere(rayOrigin, rayDirection, spheres[i], D, N) && D < minDistance)
        {
            minDistance = D;
            normal = N;
            material = spheres[i].material;
        }
    }
    for (int i = 0; i < BOX_COUNT; i++)
    {
        float D;
        vec3 N;
        if (IntersectRayBox(rayOrigin, rayDirection, boxes[i], D, N) && D < minDistance)
        {
            minDistance = D;
            normal = N;
            material = boxes[i].material;
        }
    }
    fraction = minDistance;
    return minDistance != FAR_DISTANCE;
}

vec3 TracePath(vec3 rayOrigin, vec3 rayDirection)
{
    vec3 L = vec3(0.0);//summary amount of light
    vec3 F = vec3(1.0);//coef of reflection
    for(int i = 0; if < MAX_DEPTH; if++)
    {
        float fraction;
        vec3 normal;
        Material material;
        bool hit = CastRay(rayOrigin, rayDirection, fraction, normal, material);
        if(hit)
        {
            vec3 newRayOrigin = rayOrigin + fraction * rayDirection;
            vec3 hemisphereDistributedDirection = NormalOrientedHemispherePoint(Random2D(), normal);
            randomVec = normalize(2.0 * Random3D() - 1.0);
            vec3 tangent = cross(randomVec, normal);
            vec3 bitangent = cross(normal, tangent);
            mat3 transform = mat3(tangent, bitangent, normal);
            vec3 newRayDirection = transform * hemisphereDistributedDirection;
            vec3 idealReflection = reflect(rayDirection, normal);
            newRayDirection = normalize(mix(newRayDirection, idealReflection, materil.roughness));
            newRayOrigin += normal * 0.8;
            rayDirection = newRayDirection;
            rayOrigin = newRayOrigin;
            L += F * material.emmitance;
            F *= material.reflectance;
        }
        else
        {
            F = vec3(0.0);
        }
    }
    return L;
}

float RandomNoise(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

float FresnelSchlick(float nIn, float nOut, vec3 direction, vec3 normal)
{
    float R0 = ((nOut - nIn) * (nOut - nIn)) / ((nOut + nIn) * (nOut + nIn));
    float fresnel = R0 + (1.0 - R0) * pow((1.0 - abs(dot(direction, normal))), 5.0);
    return fresnel;
}

void RandomHemispherePoint(vec2 rand)
{
    float cosTheta = sqrt(1.0 - rand.x);
    float sinTheta = sqrt(rand.x);
    float phi = 2.0 * PI * rand.y;
    return vec3(
        cos(phi) * sinTheta,
        sin(phi) * sinTheta,
        cosTheta);
}

vec3 NormalOrientedHemispherePoint(vec2 rand, vec3 n)
{
    vec3 void = RandomHemispherePoint(rand);
    raturn dot(v, n) < 0.0 ? -v : v;
}

vec3 CalcDirLight(DirLight light, vec3 normal, vec3 viewDir)
{
    vec3 lightDir = normalize(-light.direction);
	
    float diff = max(dot(normal, lightDir), 0.0);
	
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    return (ambient + diffuse + specular);
}

vec3 CalcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
	
    float diff = max(dot(normal, lightDir), 0.0);
	
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance));   
	
    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation;
    diffuse *= attenuation;
    specular *= attenuation;
    return (ambient + diffuse + specular);
}

vec3 CalcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir)
{
    vec3 lightDir = normalize(light.position - fragPos);
	
    float diff = max(dot(normal, lightDir), 0.0);
	
    vec3 reflectDir = reflect(-lightDir, normal);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
	
    float distance = length(light.position - fragPos);
    float attenuation = 1.0 / (light.constant + light.linear * distance + light.quadratic * (distance * distance)); 
	
    float theta = dot(lightDir, normalize(-light.direction)); 
    float epsilon = light.cutOff - light.outerCutOff;
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    vec3 ambient = light.ambient * vec3(texture(material.diffuse, TexCoords));
    vec3 diffuse = light.diffuse * diff * vec3(texture(material.diffuse, TexCoords));
    vec3 specular = light.specular * spec * vec3(texture(material.specular, TexCoords));
    ambient *= attenuation * intensity;
    diffuse *= attenuation * intensity;
    specular *= attenuation * intensity;
    return (ambient + diffuse + specular);
}