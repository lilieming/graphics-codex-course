#include "MeshHelper.h"

of3dPrimitive MeshHelper::toPrimitive(const ofMesh& mesh) {
    of3dPrimitive primitive;
    primitive.getMesh().clear();
    primitive.getMesh().append(mesh);
    primitive.getMesh().enableNormals();
    return primitive;
}

void MeshHelper::readModelAndGetPrimitives(ofxAssimpModelLoader& model,
                                               vector<of3dPrimitive>& primitives,
                                               ofNode& parentNode){
    for (int i = 0; i< model.getMeshCount(); i++) {
        auto primitive = MeshHelper::toPrimitive(model.getMesh(i));
        primitive.setParent(parentNode);
        primitives.push_back(primitive);
    };
}
