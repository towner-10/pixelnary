FROM ubuntu:latest

# Install premake5
RUN apt update && apt install -y build-essential curl

# Download the premake5 tarball
RUN curl -L https://github.com/premake/premake-core/releases/download/v5.0.0-beta2/premake-5.0.0-beta2-linux.tar.gz -o premake.tar.gz && \
    tar -xzf premake.tar.gz && \
    mv premake5 /usr/local/bin/premake5 && \
    rm premake.tar.gz

# Copy the project files
COPY . /app

# Set the working directory
WORKDIR /app

# Generate the project files
RUN premake5 gmake2

# Build the project
RUN make config=release

# Expose the port
EXPOSE 25565

# Run the server
CMD ["./bin/Release/ScribbleServer"]
