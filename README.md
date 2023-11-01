# Computer Vision Object Detection System

## Overview

The system provides real-time object detection in images using the YOLO model. The client, deployable on any platform including Raspberry Pi via Docker, captures and transmits images. These images are then processed on an AWS-hosted server which identifies specific objects and can optionally relay detected images directly to a Telegram account.

## System Architecture

### Client

The client, which can be deployed on platforms like Raspberry Pi, is responsible for capturing images and sending them for processing. Its main components include:

1. **Camera Module**: This facilitates interaction with the physical camera.
2. **Video Source**: Manages video streams ensuring high-quality image capture.
3. **gRPC Image Writer**: Handles the transmission of images to the server via gRPC.

Moreover, the client's primary process utilizes two threads:
- One thread reads from the camera and pushes to a queue.
- The second thread pulls from the queue and sends to the server.

### Server

Situated on AWS but also compatible with Windows, the server processes and analyzes the images. Key components include:

1. **gRPC Image Reader**: This receives the images from the client.
2. **YOLO Backend**: Uses the YOLO model to process and identify objects in the images.
3. **Database Interface**: This stores and retrieves image data and detection results.

The server's main process utilizes three threads:
- The first reads from the client and pushes images to a queue.
- The second pulls images from the queue, processes them, and then pushes the results to a results queue (a separate queue).
- The third thread pulls processed data from the results queue and writes to a database or sends to a Telegram channel.

## Design Principles and Patterns

The architecture abides by several design principles, ensuring modularity, extensibility, and maintainability:

1. **Abstraction and Interfaces**: Interfaces such as `ICamera` and `IBackend` abstract functionalities, allowing the system to evolve without major code refactoring.
2. **Dependency Inversion**: The system is designed so that high-level modules are not dependent on low-level modules, ensuring a flexible and scalable codebase.
3. **Design Patterns**: The system incorporates the Singleton (e.g., in `configJson.cpp`) and Factory patterns, and templates (e.g., in the SafeQueue) to address recurring design problems.
4. **Thread Management**: Efficiently handling real-time data streams, the system employs threads in both client and server processes for parallel processing, ensuring speed and responsiveness.

## Technologies

- **C++**: Serves as the foundation of the system, offering performance and flexibility.
- **Linux**: The operating system underpinning many components of the system, known for its reliability and open-source nature.
- **AWS**: Provides a robust environment for the server component, ensuring scalability and reliability.
- **Docker**: Facilitates consistent deployments across various environments, from local development to cloud-based production.
- **gRPC**: Handles efficient communication between the client and server, promoting robust data transfer.
- **Raspberry Pi**: Offers a versatile platform for the client, catering to a variety of deployment scenarios.
- **CMake**: A cross-platform build system, aiding in compiling and managing the project.

## UML Diagram of System Architecture

*In green - the communication between the processes

<br>

![Untitled-2023-10-31-2035](https://github.com/Haiku54/Mobileye_Embedded_Bootcamp/assets/80857560/ca1e3af9-6d51-4db6-a511-39cbfafcd3c5)






