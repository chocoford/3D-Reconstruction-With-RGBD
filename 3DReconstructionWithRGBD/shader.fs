#version 330 core
out vec4 FragColor;

in vec3 outColor;
in vec3 normal;
in vec3 fragPos;
in vec3 worldPos;
in vec3 dkModeLightPos;
in vec2 dkModeHasLight; //(0, 0) represent false. (1, 1)means true.

uniform vec3 viewPos; 
uniform int displayMode;


void main() {
    vec3 lightPos = vec3(0); 
    vec3 lightColor = vec3(0.9, 0.9, 0.9);
    vec3 objectColor = vec3(0.4);
    float ambientStrength = 0.1;
    switch(displayMode) {
        case 0:
            FragColor = vec4(vec3(-(worldPos.z / 12.5)), 1.0);
            break;

        case 1:
            FragColor = vec4(sqrt(vec3(-(worldPos.z / 2.5) + 0.2)) * normalize(vec3(pow(normal.x, 3), normal.y, sin(normal.z))), 1.0f);
            break;

	    case 2:
            lightPos = vec3(2, 12, 2); 

            vec3 ambient = ambientStrength * lightColor;
  	
            // diffuse 
            vec3 norm = normalize(normal);
            vec3 lightDir = normalize(lightPos - fragPos);
            float diff = max(dot(norm, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
    
            // specular
            float specularStrength = 0.5;
            vec3 viewDir = normalize(viewPos - fragPos);
            vec3 reflectDir = reflect(-lightDir, norm);  
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = specularStrength * spec * lightColor;  
        
            vec3 result = (ambient + diffuse + specular) * objectColor;
            FragColor = vec4(result, 1.0); 
            break;

	    case 3:
            FragColor = vec4(outColor, 1.0);
            break;

        case 4: //dark mode
             lightPos = dkModeLightPos; 

             ambient = ambientStrength * lightColor;
  	
            // diffuse 
             norm = normalize(normal);
             lightDir = normalize(lightPos - fragPos);
             diff = max(dot(norm, lightDir), 0.0);
             diffuse = diff * lightColor;
    
            // specular
             specularStrength = 0.5;
             viewDir = normalize(viewPos - fragPos);
             reflectDir = reflect(-lightDir, norm);  
             spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
             specular = specularStrength * spec * lightColor;  
        
            if (distance(dkModeHasLight, vec2(1)) <= 0.01) result = (ambient + diffuse + specular) * objectColor;
            else result = ambient * objectColor;
            FragColor = vec4(result, 1.0); 
            break;
    }
    
    //FragColor = vec4(sqrt(outColor) * normalize(vec3(pow(normal.x, 3), normal.y, sin(normal.z))), 1.0f);
}