from dronekit import connect, VehicleMode
import time 

# connect
print("connecting ....")
vehicle = connect('127.0.0.1:14550', wait_ready=True)
print("Connected .")

#Arm and Takeoff
#Arming -  activating the motors so the drone is powered and ready to fly
def arm_and_takeoff(target_altitude):
    vehicle.mode = VehicleMode("GUIDED")
    vehicle.armed = True

    while not vehicle.armed:
        print("Waiting for arming...")
        time.sleep(1)

    print(f"Taking off to {target_altitude} meters...")
    vehicle.simple_takeoff(target_altitude)

    while True:
        alt = vehicle.location.global_relative_frame.alt
        print(f"Altitude: {alt:.2f}")

        if alt >= target_altitude * 0.95:
            print("Reached target altitude!")
            break

        time.sleep(1)


# main 
try:
    #  Takeoff to 6m
    arm_and_takeoff(6)
    #  Hover
    print("Hovering...")
    time.sleep(10)   #for 10 seconds
    # 3. Land
    print("Landing...")
    vehicle.mode = VehicleMode("LAND")

    time.sleep(5)

finally:
    print("Closing connection")
    vehicle.close()

# movement function 
