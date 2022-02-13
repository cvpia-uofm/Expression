# Expression: A dyadic conversation aid using Google Glass for people who are blind or visually impaired

## Background
Expression assists people with visual impairments in perceiving  social  signals  during a natural dyadic conversation. It performs following tasks:
1. Detect and track faces
2. Detect facial and behavioral expression (smile, yawn, sleepy etc.)
3. Detect head movements (look up/down, look left/right, tiltleft/right)
4. Provide audio feedbacks to user

![System architecture](misc/system_arch.jpg?raw=true "System Architecture")

Details can be found in this article https://web.archive.org/web/20190430120824id_/https://eudl.eu/pdf/10.4108/icst.mobicase.2014.257780

## Requiremets
1. Google glass
2. Android Studio
3. C++
4. Visual Studio 
5. CLM-Z face tracker https://github.com/TadasBaltrusaitis/CLM-framework 
6. OpenCv
## How to use:
1. Build Expression app from "app" folder using android studio
2. Install the app in Google Glass
3. Download CLM-Z and build server code from "server_code" folder using visual studio
5. Run expression "mainExpression.cpp"
6. Start app in Google Glass  
