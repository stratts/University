package rbs.booking;

import java.util.ArrayList;
import java.util.List;

/**
 * Contains and manages all the locations stored in the system
 * 
 * @author Stratton Sloane
 *
 */
public class LocationManager {

	private List<Location> locations;

	/**
	 * Constructs a location manager, initially containing no locations
	 */
	public LocationManager() {
		locations = new ArrayList<Location>();
	}

	/**
	 * Creates a new location object and stores it in the LocationManager
	 * 
	 * @param name
	 * @return A Location object with the given name
	 */
	public Location addLocation(String name) {
		Location v = new Location(name);
		locations.add(v);
		return v;
	}

	/**
	 * 
	 * @param name
	 * @return The Location object with the given name
	 */
	public Location getLocation(String name) {
		for (Location v : locations) {
			if (v.getName().equals(name))
				return v;
		}

		return null;
	}

	/**
	 * 
	 * @return A list containing all locations stored in the manager
	 */
	public List<Location> getLocations() {
		return locations;
	}

}
