#include "RayCaster.h"

RayCaster::RayCaster(const ofMesh& _mesh, glm::mat4 _globalTransfMatrix, vector<ofLight> _lights){
    mesh = _mesh;
    globalTransfMatrix = _globalTransfMatrix;
    lights = _lights;
};

// C++ Ray Casting implementation following http://graphicscodex.com

void RayCaster::traceImage(const PinholeCamera& camera, shared_ptr<ofImage>& image) const{
    const int width = int(image->getWidth());
    const int height = int(image->getHeight());

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            glm::vec3 P;
            glm::vec3 w;
            // Find the ray through (x, y) and the center of projection
            camera.getPrimaryRay(float(x) + 0.5f, float(y) + 0.5f, width, height, P, w);
            image->setColor(x, y, L_i(Ray(P, w)));
        }
    }
    image->update();
}

// Debugging implementation that computes white if there is any surface on this ray and black when there is not.
// This method return the incoming light for X. The radiance need to be calculated properly, for now it is just black and white,
// In the future it will be
/*
from the book: The first one is easy: iterate over the lights and multiply three values: the biradiance from the light, the value of the scattering distribution function, and the cosine of the angle of incidence (a dot product).
*/

ofColor RayCaster::L_i(const Ray& ray) const{
    // for all the triangles in a mesh
    // Find the first intersection (and the closest!) with the scene

    const shared_ptr<Surfel>& surfelY = findFirstIntersection(ray, this->mesh);

    //if (notNull(s)) TODO, if a ray is found, create a Surfel
    if (surfelY) {
        return L_0(surfelY, -ray.direction);
    } else {
        return ofColor(0,0,0);
    }
}

// Compute the light leaving Y, which is the same as
// the light entering X when the medium is non-absorptive

ofColor RayCaster::L_0(const shared_ptr<Surfel>& surfelY, const glm::vec3 wo) const{
    // as emitted Radiance is 0, for now, I will just caclulate the direct scattered radiance
    //return surfelX->emittedRadiance(wo) + L_scatteredDirect(surfelX, wo);
    return L_scatteredDirect(surfelY, wo);
}

/*
 From the chapter "Direct Illumination":
 Note that actual parameter surfelY is now called surfelX in method L_scatteredDirect.
 I changed the frame of reference by advancing one step closer to the light along a
 transport path. As in all equations so far, X is the point at which radiance is being
 scattered and Y is the next node closer to the light on the light transport path.
*/
ofColor RayCaster::L_scatteredDirect(const shared_ptr<Surfel>& surfelX,const glm::vec3 wo) const{
    // TODO, iterate through the lights
    glm::vec3 lightPos = lights[0].getGlobalPosition();
    //lambertian light
    glm::vec3 lightDirection = glm::normalize(lightPos - surfelX->getPosition());
    glm::vec3 color = surfelX->getColor();
    float dProd = glm::dot(surfelX->getGeometricNormal(), lightDirection);
    glm::vec3 tmpCol = glm::vec3( dProd ) * color;
    return ofColor(tmpCol.x*255, tmpCol.y*255, tmpCol.z*255);
};

// This method find the first intersection between a ray and a mesh. (TODO: check if it is really the first)
// If an intersection is founded, it returns a surfel, otherwise null.
shared_ptr<Surfel> RayCaster::findFirstIntersection(const Ray& ray, const ofMesh& mesh) const{
    vector<ofMeshFace> faces = mesh.getUniqueFaces();
    bool found = false;
    for (ofMeshFace face : faces) {
        glm::vec3 baricenter;
        found = glm::intersectRayTriangle(
                                          ray.origin, ray.direction,
                                          glm::vec3(globalTransfMatrix * glm::vec4(face.getVertex(0), 1.f)),
                                          glm::vec3(globalTransfMatrix * glm::vec4(face.getVertex(1), 1.f)),
                                          glm::vec3(globalTransfMatrix * glm::vec4(face.getVertex(2), 1.f)),
                                          baricenter);
        if (found) {
            glm::vec3 faceNormal = face.getFaceNormal();
            glm::vec3 position = getPointOnTriangle(ray, baricenter);
            return shared_ptr<Surfel>(new Surfel(faceNormal, ray.direction, position));
            break;
        }
    }
    return nullptr;
};

/*
 This method takes as argument a ray and the baricentric coordinates and returns
 the exact point on the triangle where the intersection happened.
 The variable values that are stored in the baryPosition vector need to be
 explained, because there is no documentation in the glm website for the
 glm::intersectRayTriangle method, but just in this github issue 
 https://github.com/g-truc/glm/issues/6

 the baryPosition output uses barycentric coordinates for the x and y components.
 The z component is the scalar factor for ray.

 That is,
 1.0 - baryPosition.x - baryPosition.y = actual z barycentric coordinate

 if you compute the point inside the triangle that corresponds to those barycentric coordinates, you'll find the exact same answer as if you did:
 origin + direction * baryPosition.z
*/

glm::vec3 RayCaster::getPointOnTriangle(const Ray& _ray, const glm::vec3& _baryPosition) const {
    return _ray.origin + (_ray.direction * _baryPosition.z);
};

/* It find the intersection between a ray (with origin P and direction w) and the scene.
 If ray P + tw hits triangle V[0], V[1], V[2], then the function returns true, stores the barycentric coordinates in b[],
 and stores the distance to the intersection in t. Otherwise returns false and the other output parameters are undefined.*/

