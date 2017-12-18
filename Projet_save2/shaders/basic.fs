/*!\file basic.fs
 *
 * \brief rendu avec lumière directionnelle diffuse et couleur.
 * \author Farès BELHADJ, amsi@ai.univ-paris8.fr
 * \date April 15 2016
 */
#version 330
uniform mat4 modelViewMatrix;
uniform vec4 couleur;
uniform vec4 lumPos;
uniform int fond;
uniform sampler2D myTexture;

in  vec3 vsoNormal;
in  vec3 vsoModPos;
in  vec2 vsoTexCoord;
out vec4 fragColor;

void main(void) {
  if(fond != 0) {
    fragColor = texture(myTexture, vsoTexCoord);
  } else {
    vec3 N = normalize(vsoNormal);
    vec3 L = normalize(lumPos.xyz);
    float diffuse = dot(N, -L);
    fragColor = vec4((couleur.rgb * diffuse), couleur.a);
  }
}
