#!/usr/bin/env python

# IMPORT

from __future__ import print_function
#import gspread
#from oauth2client.service_account import ServiceAccountCredentials
import sys
import os
import time
from abc import abstractmethod
from bluepy.btle import BTLEException

from blue_st_sdk.manager import Manager
from blue_st_sdk.manager import ManagerListener
from blue_st_sdk.node import NodeListener
from blue_st_sdk.feature import FeatureListener
from blue_st_sdk.features.feature_audio_adpcm import FeatureAudioADPCM
from blue_st_sdk.features.feature_audio_adpcm_sync import FeatureAudioADPCMSync
from subprocess import call

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
# Implementation of the interface used by the Feature class to notify that a
# feature has updated its data.
#
class MyFeatureListener(FeatureListener):

    num = 0
    #
    # To be called whenever the feature updates its data.
    #
    # @param feature Feature that has updated.
    # @param sample  Data extracted from the feature.
    #
    def on_update(self, feature, sample):
        if(self.num < NOTIFICATIONS):
            print(feature)
            self.num += 1


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

            while True:
                # Getting features.
                print('\nFeatures:')
                i = 1
                features = device.get_features()

                for feature in features:
                  if feature.get_name() == "Temperature":
                    print("Temperature feature found")
                    # exit_code = call("python3 sendsTemperatureAndHumidityToGoogleSpreadsheets.py", shell=True)
                  else:
                    print('%d) %s' % (i, feature.get_name()))
                  i+=1

                # Selecting a feature.
                while True:
                    choice = int(input('\nSelect a feature '
                                       '(\'0\' to disconnect): '))
                    if choice >= 0 and choice <= len(features):
                        break
                if choice == 0:
                    # Disconnecting from the device.
                    print('\nDisconnecting from %s...' % (device.get_name()))
                    device.disconnect()
                    print('Disconnection done.')
                    device.remove_listener(node_listener)
                    # Reset discovery.
                    manager.reset_discovery()
                    # Going back to the list of devices.
                    break
                feature = features[choice - 1]
                
                # Enabling notifications.
                feature_listener = MyFeatureListener()
                feature.add_listener(feature_listener)
                device.enable_notifications(feature)
                
                # Getting notifications.
                n = 0
                while n < NOTIFICATIONS:
                    if device.wait_for_notifications(0.05):
                        n += 1

                # Disabling notifications.
                device.disable_notifications(feature)
                feature.remove_listener(feature_listener)

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