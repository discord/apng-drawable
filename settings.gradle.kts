rootProject.name = "apng-drawable-root"

pluginManagement {
    repositories {
        gradlePluginPortal()
        google()
        maven { url = uri("https://jitpack.io") }
    }
}

include(":apng-drawable")
include(":sample-app")

project(":apng-drawable").name = "apng-drawable"
project(":sample-app").name = "sample-app"
