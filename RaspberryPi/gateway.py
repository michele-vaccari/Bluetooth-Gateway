#!/usr/bin/env python

# IMPORT
from __future__ import print_function
import sys
import os
import time
from abc import abstractmethod
from bluepy.btle import BTLEException

from blue_st_sdk.manager import Manager
from blue_st_sdk.manager import ManagerListener
from blue_st_sdk.node import NodeListener
from blue_st_sdk.feature import FeatureListener
from blue_st_sdk.features.feature_temperature import FeatureTemperature
from blue_st_sdk.features.feature_humidity import FeatureHumidity

import gspread
from oauth2client.service_account import ServiceAccountCredentials

# CONSTANTS
PRESENTATION_MESSAGE = """#####################
# Bluetooth Gateway #
#####################"""

SCANNING_TIME_IN_SECONDS = 5

# Number of notifications to get before disabling them.
NOTIFICATIONS = 10

# FUNCTIONS
def printPresentationMessage():
    print('\n' + PRESENTATION_MESSAGE + '\n')

# INTERFACES

#
# Implementation of the interface used by the Manager class to notify that a new
# node has been discovered or that the scanning starts/stops.
#
class MyManagerListener(ManagerListener):

    #
    # This method is called whenever a discovery process starts or stops.
    #
    # @param manager Manager instance that starts/stops the process.
    # @param enabled True if a new discovery starts, False otherwise.
    #
    def on_discovery_change(self, manager, enabled):
        print('Discovery %s.' % ('started' if enabled else 'stopped'))
        if not enabled:
            print()

    #
    # This method is called whenever a new node is discovered.
    #
    # @param manager Manager instance that discovers the node.
    # @param node    New node discovered.
    #
    def on_node_discovered(self, manager, node):
        print('New device discovered: %s.' % (node.get_name()))


#
# Implementation of the interface used by the Node class to notify that a node
# has updated its status.
#
class MyNodeListener(NodeListener):

    #
    # To be called whenever a node changes its status.
    #
    # @param node       Node that has changed its status.
    # @param new_status New node status.
    # @param old_status Old node status.
    #
    def on_status_change(self, node, new_status, old_status):
        print('Device %s from %s to %s.' %
            (node.get_name(), str(old_status), str(new_status)))


#
# Implementation of the interface used by the Feature class to notify that temperature
# feature has updated its data.
#
class TemperatureFeatureListener(FeatureListener):

    scope = ['https://spreadsheets.google.com/feeds','https://www.googleapis.com/auth/drive']
    credentials = ServiceAccountCredentials.from_json_keyfile_name('credentials.json', scope)
    client = gspread.authorize(credentials)
    temperatureSheet = client.open("SensorData").worksheet("Temperature")

    #
    # To be called whenever the feature updates its data.
    #
    # @param feature Feature that has updated.
    # @param sample  Data extracted from the feature.
    #
    def on_update(self, feature, sample):
        print(feature)
        temperature = FeatureTemperature.get_temperature(sample)
        currentTime = time.localtime()
        timeToString = time.strftime("%m/%d/%Y %H:%M:%S", currentTime)
        self.temperatureSheet.append_row([timeToString,float(temperature)])
        print('Sent the following data: ', timeToString, float(temperature))

#
# Implementation of the interface used by the Feature class to notify that humidity
# feature has updated its data.
#
class HumidityFeatureListener(FeatureListener):

    scope = ['https://spreadsheets.google.com/feeds','https://www.googleapis.com/auth/drive']
    credentials = ServiceAccountCredentials.from_json_keyfile_name('credentials.json', scope)
    client = gspread.authorize(credentials)
    humiditySheet = client.open("SensorData").worksheet("Humidity")
    
    #
    # To be called whenever the feature updates its data.
    #
    # @param feature Feature that has updated.
    # @param sample  Data extracted from the feature.
    #
    def on_update(self, feature, sample):
        print(feature)
        humidity = FeatureHumidity.get_humidity(sample)
        currentTime = time.localtime()
        timeToString = time.strftime("%m/%d/%Y %H:%M:%S", currentTime)
        self.humiditySheet.append_row([timeToString,float(humidity)])
        print('Sent the following data: ', timeToString, float(humidity))


# MAIN APPLICATION
def main(argv):

    printPresentationMessage()

    try:
        # Creating Bluetooth Manager.
        manager = Manager.instance()
        manager_listener = MyManagerListener()
        manager.add_listener(manager_listener)

        while True:
            # Synchronous discovery of Bluetooth devices.
            print('Scanning Bluetooth devices...\n')
            manager.discover(SCANNING_TIME_IN_SECONDS)

            # Getting discovered devices.
            discovered_devices = manager.get_nodes()

            # Listing discovered devices.
            if not discovered_devices:
                print('\nNo Bluetooth devices found.')
                sys.exit(0)

            print('\nAvailable Bluetooth devices:')
            i = 1
            for device in discovered_devices:
                print('%d) %s: [%s]' % (i, device.get_name(), device.get_tag()))
                i += 1

            # Selecting a device.
            while True:
                choice = int(input("\nSelect a device (\'0\' to quit): "))
                if choice >= 0 and choice <= len(discovered_devices):
                    break
            if choice == 0:
                # Exiting.
                manager.remove_listener(manager_listener)
                print('Exiting...\n')
                sys.exit(0)

            device = discovered_devices[choice - 1]
            
            # Connecting to the device.
            node_listener = MyNodeListener()
            device.add_listener(node_listener)
            print('\nConnecting to %s...' % (device.get_name()))
            device.connect()
            print('Connection done.')

            # Getting features.
            i = 0
            features = device.get_features()
            temperatureIndex = None
            humidityIndex = None

            for feature in features:
              if feature.get_name() == FeatureTemperature.FEATURE_NAME:
                print("Temperature feature found")
                temperatureIndex = i
              elif feature.get_name() == FeatureHumidity.FEATURE_NAME:
                print("Humidity feature found")
                humidityIndex = i
              i+=1

            if temperatureIndex is None and humidityIndex is None:
              print("No temperature or humidity feature found")
              # Disconnecting from the device.
              print('\nDisconnecting from %s...' % (device.get_name()))
              device.disconnect()
              print('Disconnection done.')
              device.remove_listener(node_listener)
              # Reset discovery.
              manager.reset_discovery()
              # Going back to the list of devices.
              break

            # Enabling notifications for temperature data
            if temperatureIndex is not None:
              temperatureFeature = features[temperatureIndex]
              temperatureListener = TemperatureFeatureListener()
              temperatureFeature.add_listener(temperatureListener)
              device.enable_notifications(temperatureFeature)
            
            # Enabling notifications for humidity data
            if humidityIndex is not None:
              humidityFeature = features[humidityIndex]
              humidityListener = HumidityFeatureListener()
              humidityFeature.add_listener(humidityListener)
              device.enable_notifications(humidityFeature)

            # Getting notifications.
            print('\nStart getting notifications\n')
            while True:
              device.wait_for_notifications(0.05)

    except BTLEException as e:
        print(e)
        # Exiting.
        print('Exiting...\n')
        sys.exit(0)
    except KeyboardInterrupt:
        try:
            # Exiting.
            print('\nExiting...\n')
            sys.exit(0)
        except SystemExit:
            os._exit(0)


if __name__ == "__main__":

    main(sys.argv[1:])