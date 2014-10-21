using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using Microsoft.Xna.Framework;

namespace RCClient.ResolutionIndependence
{
    class Camera2D
    {
        private float zoom;
        private float rotation;
        private Vector2 position;
        private Matrix transform = Matrix.Identity;
        private bool isViewTransformationDity = true;
        private Matrix camTranslationMatrix = Matrix.Identity;
        private Matrix camRotationMatrix = Matrix.Identity;
        private Matrix camScaleMatrix = Matrix.Identity;
        private Matrix resTranslationMatrix = Matrix.Identity;

        protected ResolutionIndependentRenderer resolutionIndependentRenderer;
        private Vector3 camTranslationVector = Vector3.Zero;
        private Vector3 camScaleVector = Vector3.Zero;
        private Vector3 resTranslationVector = Vector3.Zero;


    }
}
