package rbs.booking;

import java.util.List;
import java.util.ArrayList;
import java.time.LocalDate;

/**
 * A location with one or more rooms and a unique name. Also reponsible for
 * checking for and returning available rooms
 * 
 * @author Stratton Sloane
 *
 */
public class Location {

	private String name;
	private List<Room> rooms;

	/**
	 * Constructs a new location with the given name, initially with no rooms
	 * 
	 * @param name A unique name
	 */
	public Location(String name) {
		this.name = name;
		this.rooms = new ArrayList<Room>();
	}

	/**
	 * 
	 * @return The name of the location
	 */
	public String getName() {
		return name;
	}

	/**
	 * Adds a new room to the location
	 * 
	 * @param room The Room object to add
	 */
	public void addRoom(Room room) {
		this.rooms.add(room);
	}

	/**
	 * Returns the room with the given name
	 * 
	 * @param name The name of the room
	 * @return A Room object, or null if the name doesn't exist
	 */
	public Room getRoom(String name) {
		for (Room r : rooms) {
			if (r.getName().equals(name))
				return r;
		}

		return null;
	}

	/**
	 * Returns a list containing all the rooms in the location
	 * 
	 * @return A list of Room objects
	 */
	public List<Room> getRooms() {
		return rooms;
	}

	/**
	 * Given a start date, end date, and required rooms, determines whether the
	 * location can fulfill the reservation
	 * 
	 * @param startDate The desired start date
	 * @param endDate   The desired end date
	 * @param small     The number of small rooms required
	 * @param medium    The number of medium rooms required
	 * @param large     The number of large rooms required
	 * @return True if the location can fulfill the requirements, false if not
	 */
	public boolean hasAvailableRooms(LocalDate startDate, LocalDate endDate, int small, int medium, int large) {
		int smallRooms = 0, mediumRooms = 0, largeRooms = 0;

		// Count each available room size
		for (Room r : rooms) {
			if (r.isAvailable(startDate, endDate)) {
				switch (r.getSize()) {
					case Small:
						smallRooms++;
						break;
					case Medium:
						mediumRooms++;
						break;
					case Large:
						largeRooms++;
						break;
				}
			}
		}

		if (smallRooms >= small && mediumRooms >= medium && largeRooms >= large) {
			return true;
		} else
			return false;
	}

	/**
	 * Returns a list containing all rooms within the location that are available in
	 * the given date range
	 * 
	 * @param startDate The desired start date
	 * @param endDate   The desired end date
	 * @return The list of available rooms, as Room objects
	 */
	public List<Room> getAvailableRooms(LocalDate startDate, LocalDate endDate) {
		List<Room> available = new ArrayList<Room>();

		for (Room r : rooms) {
			if (r.isAvailable(startDate, endDate))
				available.add(r);
		}

		return available;
	}
}
