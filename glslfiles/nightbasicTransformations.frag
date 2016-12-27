#version 150

in  vec2 ex_TexCoord; //texture coord arriving from the vertex
in  vec3 ex_Normal;  //normal arriving from the vertex

out vec4 out_Color;   //colour for the pixel
in vec3 ex_LightDir1;  //light direction arriving from the vertex
in vec3 ex_LightDir2;  //light direction arriving from the vertex

in vec3 ex_PositionEye;

uniform vec4 light_ambient;
uniform vec4 light_diffuse;
uniform vec4 light_specular;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_shininess;
uniform vec3 LightFor;

uniform sampler2D DiffuseMap;

void main(void)
{

	//out_Color = texture(DiffuseMap, ex_TexCoord); //show texture values

	//out_Color = vec4(ex_Normal,1.0); //Show normals

	//out_Color = vec4(ex_TexCoord,0.0,1.0); //show texture coords

	//Calculate lighting
	vec3 n, L1, L2, f;
	vec4 color;
	float NdotL;
	
	f = normalize(LightFor);
	n = normalize(ex_Normal);
	L1 = normalize(ex_LightDir1);
	L2 = normalize(ex_LightDir2);

	vec3 v = normalize(-ex_PositionEye);
	vec3 r = normalize(-reflect(L1, n));
	
	float RdotV = max(0.0, dot(r, v));

	

	color = light_ambient * material_ambient;
	
	NdotL = dot(f, -L1);
	if(NdotL > 0.9){
			NdotL = max(dot(n, L1),0.0);
			color += (light_diffuse * material_diffuse * NdotL);
	}

	NdotL = dot(f, -L2);
	if(NdotL > 0.9){
			NdotL = max(dot(n, L2),0.0);
			color += (light_diffuse * material_diffuse * NdotL);
	}

	//color += material_specular * light_specular * pow(RdotV, material_shininess);

	//out_Color = color;  //show just lighting

    out_Color = color * texture(DiffuseMap, ex_TexCoord); //show texture and lighting
}