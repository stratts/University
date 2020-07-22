package rbs.booking;

import java.time.LocalDate;
import java.util.List;
import java.util.ArrayList;

/**
 * Represents a single reservation of one or more rooms within a location
 * 
 * @author Stratton Sloane
 *
 */
public class Reservation {

	private String id;
	private LocalDate startDate;
	private LocalDate endDate;
	private String location;
	private List<String> rooms;

	/**
	 * 
	 * @param id        The ID associated with the reservation
	 * @param startDate Date the reservation begins
	 * @param endDate   Date the reservation ends
	 */
	public Reservation(String id, LocalDate startDate, LocalDate endDate) {
		this.id = id;
		this.startDate = startDate;
		this.endDate = endDate;
		this.rooms = new ArrayList<String>();
	}

	/**
	 * 
	 * @return The ID of the reservation
	 */
	public String getID() {
		return id;
	}

	/**
	 * 
	 * @return The date the reservation starts, in LocalDate format
	 */
	public LocalDate getStartDate() {
		return startDate;
	}

	/**
	 * 
	 * @return The date the reservation ends, in LocalDate format
	 */
	public LocalDate getEndDate() {
		return endDate;
	}

	/**
	 * 
	 * @return The name of the location that the reservation is located in
	 */
	public String getLocation() {
		return location;
	}

	/**
	 * 
	 * @param location The location that the reservation is located in
	 */
	public void setLocation(String location) {
		this.location = location;
	}

	/**
	 * 
	 * @return A list containing all rooms reserved
	 */
	public List<String> getRooms() {
		return rooms;
	}

	/**
	 * 
	 * @param room The name of the room that has been reserved
	 */
	public void addRoom(String room) {
		rooms.add(room);
	}

}
